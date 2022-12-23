#include <ngx_http.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "lib/admin.h"
#include "lib/init_shm.h"

typedef struct {
    ngx_shm_zone_t *shm_zone;
    ngx_flag_t enabled;
    ngx_str_t IP;
} ngx_http_stats_loc_conf_t;


static ngx_int_t ngx_http_stats(ngx_conf_t *cf);
static void *ngx_http_stats_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_stats_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static ngx_int_t ngx_http_stats_handler(ngx_http_request_t *r);

static ngx_int_t server_html_stats(ngx_http_request_t *r);
static ngx_int_t server_html_stats_r_p_s(ngx_http_request_t *r);




static ngx_command_t ngx_http_stats_commands[] = {
        {
                ngx_string("ngx_stats"),
                NGX_HTTP_LOC_CONF | NGX_HTTP_LIF_CONF | NGX_HTTP_SIF_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_FLAG,
                ngx_conf_set_flag_slot,
                NGX_HTTP_LOC_CONF_OFFSET,
                offsetof(ngx_http_stats_loc_conf_t, enabled),
                NULL
        },
        {
                ngx_string("ngx_stats_admin_ip"),
                NGX_HTTP_LOC_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_TAKE1,
                ngx_conf_set_str_slot,
                NGX_HTTP_LOC_CONF_OFFSET,
                offsetof(ngx_http_stats_loc_conf_t, IP),
                NULL
        },
        ngx_null_command
};


static ngx_http_module_t ngx_http_stats_module_ctx = {
        NULL, 
        ngx_http_stats, 
        NULL,
        NULL,
        NULL,
        NULL,
        ngx_http_stats_create_loc_conf,
        ngx_http_stats_merge_loc_conf
};

ngx_module_t ngx_http_stats_module = {
        NGX_MODULE_V1,
        &ngx_http_stats_module_ctx,
        ngx_http_stats_commands,
        NGX_HTTP_MODULE,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NGX_MODULE_V1_PADDING
};


static void *ngx_http_stats_create_loc_conf(ngx_conf_t *cf) {
    ngx_http_stats_loc_conf_t *conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_stats_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    conf->IP = (ngx_str_t) {0, NULL};
    conf->enabled = NGX_CONF_UNSET;

    return conf;
}


static char *ngx_http_stats_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child) {
    ngx_http_stats_loc_conf_t *prev = parent;
    ngx_http_stats_loc_conf_t *conf = child;

    ngx_conf_merge_value(conf->enabled, prev->enabled, 0)

    ngx_conf_merge_str_value(conf->IP, prev->IP, NULL)
    ngx_shm_zone_t *shm_zone;
    ngx_str_t *shm_name;
    shm_name = ngx_palloc(cf->pool, sizeof *shm_name);
    shm_name->len = sizeof("shared_memory") - 1;
    shm_name->data = (unsigned char *) "shared_memory";
    shm_zone = ngx_shared_memory_add(cf, shm_name,  8*ngx_pagesize, &ngx_http_stats_module); 
    if(shm_zone == NULL){
        return NGX_CONF_ERROR;
    }

    shm_zone->init = ngx_stats_init_shm_zone;
    conf->shm_zone = shm_zone;

    ngx_conf_merge_ptr_value(conf->shm_zone, prev->shm_zone, NULL);

    return NGX_CONF_OK;
}


static ngx_int_t ngx_http_stats(ngx_conf_t *cf) {
    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *main_conf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&main_conf->phases[NGX_HTTP_PRECONTENT_PHASE].handlers);
    if (h == NULL) {
        ngx_log_error(NGX_LOG_ERR, cf->log, 0, "null");
        return NGX_ERROR;
    }

    *h = ngx_http_stats_handler;

    return NGX_OK;
}


static ngx_int_t ngx_http_stats_handler(ngx_http_request_t *r) {
    
    ngx_http_stats_loc_conf_t *conf = ngx_http_get_module_loc_conf(r, ngx_http_stats_module);
    
    if (!conf->enabled) {
        return NGX_DECLINED;
    }

    unsigned char  key_s;
    unsigned int mTime;
    ngx_http_stats_loc_conf_t *lccf;
    ngx_shm_zone_t *shm_zone;

    lccf = ngx_http_get_module_loc_conf(r, ngx_http_stats_module);

    if(lccf->shm_zone == NULL){
      return NGX_DECLINED;
    }
    shm_zone = lccf->shm_zone;
    mTime = time(NULL);
    key_s = mTime % 10;
    
    ngx_shmtx_lock(&shpool->mutex);
    //count req/s
    if ((mTime - 9 )> ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[key_s].time ){
      ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[key_s].time = mTime;
      ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[key_s].count = 0;
    }
    ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[key_s].count ++;
    ngx_shmtx_unlock(&shpool->mutex);

    //check if is admin
    int isAdm = is_admin(r, (char *)conf->IP.data);
    switch ( isAdm ) {
        case 1:
            return server_html_stats(r);
            
        case 2:
            return server_html_stats_r_p_s( r);
    }

    return NGX_DECLINED;
    
}



static ngx_int_t server_html_stats(ngx_http_request_t *r) {
    ngx_http_stats_loc_conf_t *lccf;
    ngx_shm_zone_t *shm_zone;
    unsigned char   key;
    unsigned int    mTime;
    ngx_buf_t* b;
    ngx_chain_t out;
    char html[8192] =" ";

    lccf = ngx_http_get_module_loc_conf(r, ngx_http_stats_module);
    if(lccf->shm_zone == NULL){
      return NGX_DECLINED;
    }

    shm_zone = lccf->shm_zone;
    mTime = time(NULL);
    key = (mTime - 9 ) % 10;

    sprintf(html,ADMIN_HTML,  
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ key].time,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+1)%10].time,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+2)%10].time,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+3)%10].time,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+4)%10].time,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+5)%10].time,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+6)%10].time,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+7)%10].time,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+8)%10].time,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+9)%10].time,
                
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ key].count,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+1)%10].count,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+2)%10].count,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+3)%10].count,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+4)%10].count,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+5)%10].count,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+6)%10].count,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+7)%10].count,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+8)%10].count,
                ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[ (key+9)%10].count 
            );

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    out.buf = b;
    out.next = NULL;
    ngx_str_t count_str = ngx_string(html);

    b->pos = count_str.data;
    b->last = count_str.data + count_str.len;
    b->memory = 1;
    b->last_buf = 1;
    r->headers_out.content_type.len = sizeof("text/html") - 1;
    r->headers_out.content_type.data = (u_char *) "text/html";
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = count_str.len;

    ngx_http_send_header(r);
    ngx_http_output_filter(r, &out);
    ngx_http_finalize_request(r, 0);

    return NGX_DONE;
}


static ngx_int_t server_html_stats_r_p_s(ngx_http_request_t *r) {
    char json[64], *json_response;
    unsigned char  key;
    unsigned int mTime;
    ngx_http_stats_loc_conf_t *lccf;
    ngx_shm_zone_t *shm_zone;
    ngx_buf_t* b;
    ngx_chain_t out;

    lccf = ngx_http_get_module_loc_conf(r, ngx_http_stats_module);

    if(lccf->shm_zone == NULL){
      return NGX_DECLINED;
    }
    shm_zone = lccf->shm_zone;
    mTime = time(NULL);
    
    key = (mTime - 1  )% 10 ; // -1 second
    //create JSON
    sprintf( json, "{\"req\":%ld, \"time\":%u}\n",
            (long int)(((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[key].count/2), 
            ((ngx_stats_shm_count_t *)shm_zone->data)->r_per_s[key].time
            );
    json_response = malloc(strlen(json)+1);
    strncpy(json_response, json, strlen(json));
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    out.buf = b;
    out.next = NULL;
  
    b->pos = (u_char *)json_response;
    b->last = (u_char *)json_response + strlen(json_response);
    b->memory = 1;
    b->last_buf = 1;
    
    r->headers_out.content_type.len = sizeof("application/json") - 1;
    r->headers_out.content_type.data = (u_char *) "application/json";
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = strlen(json_response);

    ngx_http_send_header(r);
    ngx_http_output_filter(r, &out);
    ngx_http_finalize_request(r, 0);

    free(json_response);
    return NGX_DONE;
}




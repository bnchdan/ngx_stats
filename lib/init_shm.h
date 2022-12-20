typedef struct {
    unsigned int time;
    long int count;        
} req;

typedef struct{
  req r_per_s[9];
} ngx_stats_shm_count_t;

ngx_slab_pool_t *shpool;

static ngx_int_t ngx_stats_init_shm_zone(ngx_shm_zone_t *shm_zone, void *data);
void init_r_p_s(req *data, int size);

static ngx_int_t ngx_stats_init_shm_zone(ngx_shm_zone_t *shm_zone, void *data){
  ngx_stats_shm_count_t *shm_data;
  if(data){ 
    shm_zone->data = data;
    return NGX_OK;
  }
  shpool = (ngx_slab_pool_t *)shm_zone->shm.addr;
  shm_data = ngx_slab_alloc(shpool, sizeof *shm_data  );
  init_r_p_s(shm_data->r_per_s , 9);
  shm_zone->data = shm_data;

  return NGX_OK;
}

void init_r_p_s(req *data, int size){
    for (int i=0; i<= size; i++){
        data[i].count = 0;
        data[i].time = 0 ;
    }
}




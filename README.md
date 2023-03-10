# ngx_stats

[Demo](https://bnchdan.github.io/ngx_stats/demo_ngx_stats.html)

![image](https://user-images.githubusercontent.com/30780133/208644100-b586a7e4-47bc-4c89-87e3-f98124267789.png)


### Installation

1. Add load_module ngx_http_stats_module.so; to /etc/nginx/nginx.conf
2. Restart /etc/init.d/nginx restart
3. In browser go to  [(ip/hostname)/ngx_stats](https://bnchdan.github.io/ngx_stats/demo_ngx_stats.html)
### Build from source

1. Download nginx corresponding to your current version (Check with `nginx -v`)
    ```bash
   wget https://nginx.org/download/nginx-1.16.1.tar.gz
   tar -xzf nginx-1.16.1.tar.gz
   export NGINX_PATH=$(pwd)/nginx-1.16.1/
    ```
2. Compile the module
    ```bash
    git clone https://github.com/bnchdan/ngx_stats
    cd ngx_stats
    ./build.sh
    ```
3. The dynamic module can be found at `${NGINX_PATH}/objs/ngx_http_stats_module.so`

### Simple configuration
 ```
    load_module  {your_ngx_path}/objs/ngx_http_stats_module.so;
    ...
    http {
        ...
        server{
            ...
            ngx_stats on;   
            ngx_stats_admin_ip "127.0.0.1";   
            ...
        }
        ...
    }
    ...
 
 ```
 
 

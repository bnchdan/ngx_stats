# ngx_stats



### Installation

1. Add `load_module ngx_http_js_challenge_module.so;` to `/etc/nginx/nginx.conf`
1. Restart `/etc/init.d/nginx restart`

### Build from source

1. Download nginx corresponding to your current version (Check with `nginx -v`)
    ```bash
   wget https://nginx.org/download/nginx-1.16.1.tar.gz
   tar -xzf nginx-1.16.1.tar.gz
   export NGINX_PATH=$(pwd)/nginx-1.16.1/
    ```
2. Compile the module
    ```bash
    git clone https://github.com/simon987/ngx_http_js_challenge_module
    cd ngx_http_js_challenge_module
    ./build.sh
    ```
3. The dynamic module can be found at `${NGINX_PATH}/objs/ngx_http_js_challenge_module.so`

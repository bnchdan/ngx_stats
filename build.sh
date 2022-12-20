#!/bin/bash
export NGINX_PATH=/home/debian/nginx-1.16.1

if [ -z ${NGINX_PATH+x} ]; then
  echo "Please set the NGINX_PATH variable";
  exit
fi

MODULE_PATH=$(pwd)/
CONFIG_ARGS=$(nginx -V 2>&1 | tail -n 1 | cut -c 21- | sed 's/--add-dynamic-module=.*//g')
CONFIG_ARGS="${CONFIG_ARGS} --add-dynamic-module=${MODULE_PATH}"
echo $CONFIG_ARGS

(
  cd ${NGINX_PATH} || exit
  bash -c "./configure ${CONFIG_ARGS}"
  make modules -j "$(nproc)"
) || exit

echo "Load the dynamic module ${NGINX_PATH}/objs/ngx_http_stats_module.so and restart nginx to install"


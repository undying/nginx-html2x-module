#! /bin/bash

#exec \
#  valgrind \
#    --leak-check=full \
#    --show-leak-kinds=all \
#      nginx -g 'daemon off;'

exec \
  nginx -g 'daemon off;'

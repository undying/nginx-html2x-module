#! /bin/bash

#exec \
#  valgrind \
#    --leak-check=full \
#    --show-leak-kinds=all \
#      nginx -g 'daemon off;'

exec \
  nginx -g 'daemon off;'

#exec \
#  gdb --args nginx -g 'daemon off;'

#exec \
#  /bin/bash -c 'time /opt/mod_pdf/src/html_to_pdf_test'

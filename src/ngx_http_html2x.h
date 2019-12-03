
#ifndef NGX_HTTP_HTML2X_H
#define NGX_HTTP_HTML2X_H

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "ngx_http_html2pdf.h"

/* strlen("$arg__") == 6 */
#define HTML2X_NGX_URI_ARG_LEN 5
#define HTML2X_NGX_URI_ARG_DATA "$arg_"
#define HTML2X_IS_NGX_VARIABLE(str) (str[0] == '$')


extern ngx_module_t ngx_http_html2x_module;

typedef struct {
  ngx_flag_t enable;

  wkhtmltopdf_global_t wk_gs;
  wkhtmltopdf_object_t wk_os;
} ngx_http_html2x_loc_conf_t;


ngx_http_variable_value_t * ngx_http_html2x_get_variable(ngx_http_request_t *r, ngx_str_t *name);

ngx_str_t * ngx_http_html2x_variable_value_get(ngx_http_request_t *r, ngx_str_t *name);

ngx_int_t ngx_http_html2x_is_ngx_uri_arg(ngx_str_t *var);


#endif


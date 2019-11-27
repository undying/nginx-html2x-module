
#ifndef NGX_HTTP_HTML2PDF_H
#define NGX_HTTP_HTML2PDF_H

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "ngx_http_html2pdf_wkhtmltopdf.h"


char * ngx_http_html2pdf(ngx_conf_t *ngx_conf, ngx_command_t *ngx_command, void *conf);
ngx_int_t ngx_http_html2pdf_handler(ngx_http_request_t *r);


#endif


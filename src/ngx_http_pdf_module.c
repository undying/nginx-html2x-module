
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "ngx_http_pdf_wkhtmltopdf.h"


typedef struct {
  ngx_flag_t enable;
} ngx_http_pdf_loc_conf_t;

static const char ngx_http_pdf_content_type[] = "application/pdf";

static char * ngx_http_pdf(ngx_conf_t *ngx_conf, ngx_command_t *ngx_command, void *conf);

static ngx_int_t ngx_http_pdf_preconf(ngx_conf_t *ngx_conf);
static ngx_int_t ngx_http_pdf_postconf(ngx_conf_t *ngx_conf);

static void * ngx_http_pdf_create_loc_conf(ngx_conf_t *ngx_conf);
static char * ngx_http_pdf_merge_loc_conf(ngx_conf_t *ngx_conf, void *parent, void *child);

static ngx_int_t ngx_http_pdf_handler(ngx_http_request_t *r);
static void ngx_http_pdf_request_body(ngx_http_request_t *r);
//static ngx_int_t ngx_http_pdf_init(ngx_http_pdf_loc_conf_t *pdf_loc_conf);


static ngx_command_t ngx_http_pdf_commands[] = {
  { 
    ngx_string("pdf"), 
    NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
    ngx_http_pdf,
    NGX_HTTP_LOC_CONF_OFFSET,
    0,
    NULL
  },
  ngx_null_command
};

static ngx_http_module_t ngx_http_pdf_module_ctx = {
  ngx_http_pdf_preconf, /* preconfiguration */
  ngx_http_pdf_postconf, /* postconfiguration */

  NULL, /* create main configuration */
  NULL, /* init main configuration */

  NULL, /* create server configuration */
  NULL, /* merge server configuration */

  ngx_http_pdf_create_loc_conf, /* create location configuration */
  ngx_http_pdf_merge_loc_conf /* merge location configuration */
};

ngx_module_t ngx_http_pdf_module = {
  NGX_MODULE_V1,
  &ngx_http_pdf_module_ctx,	     /* module context */
  ngx_http_pdf_commands,	       /* module directives */
  NGX_HTTP_MODULE,               /* module type */
  NULL,                          /* init master */
  NULL,                          /* init module */
  NULL,                          /* init process */
  NULL,                          /* init thread */
  NULL,                          /* exit thread */
  NULL,                          /* exit process */
  NULL,                          /* exit master */
  NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_pdf_handler(ngx_http_request_t *r)
{
  ngx_int_t rc;

  if(r->method & (NGX_HTTP_HEAD|NGX_HTTP_GET)){
    //TODO: generate documentation on GET
    return NGX_HTTP_NOT_ALLOWED;
  }

  rc = ngx_http_read_client_request_body(r, ngx_http_pdf_request_body);
  if(rc >= NGX_HTTP_SPECIAL_RESPONSE){
    return rc;
  }

  return NGX_DONE;
}

/*
static ngx_int_t
ngx_http_pdf_handler2(ngx_http_request_t *r){
  const char * test_response = "fuck yeah";
  ngx_int_t rc;

  //ngx_buf_t *b;
  //ngx_chain_t out;

  //ngx_http_pdf_loc_conf_t *pdf_loc_conf;
  //pdf_loc_conf = ngx_http_get_module_loc_conf(r, ngx_http_pdf_module);

  if(!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))){
    // TODO: allow only POST
    return NGX_HTTP_NOT_ALLOWED;
  }

  r->headers_out.content_type.len = ngx_strlen(ngx_http_pdf_content_type);
  r->headers_out.content_type.data = (u_char *)ngx_http_pdf_content_type;

  r->headers_out.status = NGX_HTTP_OK;
  r->headers_out.content_length_n = ngx_strlen(test_response);

  rc = ngx_http_send_header(r);
  if(rc == NGX_ERROR || rc > NGX_OK || r->header_only){
    return rc;
  }

  // TODO: use ngx_http_read_client_request_body() to read body
  //rc = ngx_http_discard_request_body(r);
  //if(rc != NGX_OK && rc != NGX_AGAIN){
  //  return rc;
  //}
  rc = ngx_http_read_client_request_body(r, ngx_http_pdf_request_body);
  if(rc > NGX_HTTP_SPECIAL_RESPONSE){
    return rc;
  }

  //b = ngx_calloc_buf(r->pool);
  //if(!b){
  //  ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Failed to allocate response buffer.");
  //  return NGX_HTTP_INTERNAL_SERVER_ERROR;
  //}

  //b->pos = (u_char *)test_response;
  //b->last = b->pos + ngx_strlen(test_response);

  //b->last_buf = (r == r->main) ? 1 : 0;
  //b->last_in_chain = 1;
  //b->memory = 1;

  //out.buf = b;
  //out.next = NULL;

  //return ngx_http_output_filter(r, &out);

  return NGX_DONE;
}
*/


static void ngx_http_pdf_request_body(ngx_http_request_t *r)
{
  int rc;
  ngx_buf_t *b, *fb;
  ngx_chain_t *in, out;

  pdf_conf_t pdf_conf;

  if(!r->request_body){
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
        "HTML body not passed");
    ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
    return;
  }

  pdf_init(&pdf_conf);

  fb = ngx_calloc_buf(r->pool);
  if(!fb){
    ngx_log_error(NGX_LOG_CRIT, r->connection->log, 0,
        "unable to allocate file buffer");
    ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
    return;
  }

  for(in = r->request_body->bufs; in; in = in->next){
    if(ngx_buf_in_memory(in->buf)){
      pdf_object_add(&pdf_conf, (char *)in->buf->start);
    } else if(in->buf->in_file){
      fb->start = ngx_palloc(r->pool, in->buf->file_last); /*TODO: maybe it's possible to reuse buffers instead of new alloc*/
      ngx_read_file(in->buf->file, fb->start, in->buf->file_last, in->buf->file_pos);
      pdf_object_add(&pdf_conf, (char *)fb->start);
    }
  }

  b = ngx_calloc_buf(r->pool);
  if(!b){
    ngx_log_error(NGX_LOG_CRIT, r->connection->log, 0,
        "unable to allocate buffer");
    ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
    return;
  }

  rc = pdf_convert(&pdf_conf, &b->pos);
  if(rc < 1){
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
        "unable to convert HTML to PDF");
    ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
    return;
  }

  b->last = b->pos + rc;

  b->memory = 1;
  b->last_buf = 1;

  out.buf = b;
  out.next = NULL;

  r->headers_out.status = NGX_HTTP_OK;
  r->headers_out.content_length_n = rc;
  r->headers_out.content_type.len = ngx_strlen(ngx_http_pdf_content_type);
  r->headers_out.content_type.data = (u_char *)ngx_http_pdf_content_type;

  rc = ngx_http_send_header(r);
  if(rc == NGX_ERROR || rc > NGX_OK || r->header_only){
    ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
    return;
  }

  rc = ngx_http_output_filter(r, &out);

  pdf_deinit(&pdf_conf);
  ngx_http_finalize_request(r, rc);
}


/*
static void ngx_http_pdf_request_body(ngx_http_request_t *r)
{
  off_t len = 0;
  ngx_buf_t *b;
  ngx_int_t rc;
  ngx_chain_t *in, out;

  if(!(r->request_body)){
    ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
    return;
  }

  for(in = r->request_body->bufs; in; in = in->next){
    len += ngx_buf_size(in->buf);
  }

  b = ngx_create_temp_buf(r->pool, NGX_OFF_T_LEN);
  if(!b){
    ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
    return;
  }

  b->last = ngx_sprintf(b->pos, "%0", len);
  b->last_buf = (r == r->main) ? 1 : 0;
  b->last_in_chain = 1;

  r->headers_out.status = NGX_HTTP_OK;
  r->headers_out.content_length_n = b->last - b->pos;

  rc = ngx_http_send_header(r);
  if(rc == NGX_ERROR || rc > NGX_OK || r->header_only){
    ngx_http_finalize_request(r, rc);
    return;
  }

  out.buf = b;
  out.next = NULL;

  rc = ngx_http_output_filter(r, &out);
  ngx_http_finalize_request(r, rc);
}
*/


static char * ngx_http_pdf(ngx_conf_t *ngx_conf, ngx_command_t *ngx_command, void *conf){
  ngx_http_core_loc_conf_t *ngx_core_loc_conf;
  ngx_http_pdf_loc_conf_t *pdf_loc_conf = conf;

  ngx_core_loc_conf = ngx_http_conf_get_module_loc_conf(ngx_conf, ngx_http_core_module);
  ngx_core_loc_conf->handler = ngx_http_pdf_handler;

  pdf_loc_conf->enable = 1;

  return NGX_CONF_OK;
}


static ngx_int_t ngx_http_pdf_preconf(ngx_conf_t *ngx_conf)
{
  return 0;
}


static ngx_int_t ngx_http_pdf_postconf(ngx_conf_t *ngx_conf)
{
  return 0;
}


//static ngx_int_t ngx_http_pdf_init(ngx_http_pdf_loc_conf_t *pdf_loc_conf)
//{
//  return 0;
//}


static void * ngx_http_pdf_create_loc_conf(ngx_conf_t *ngx_conf)
{
  ngx_http_pdf_loc_conf_t *pdf_loc_conf;

  pdf_loc_conf = ngx_pcalloc(ngx_conf->pool, sizeof(ngx_http_pdf_loc_conf_t));
  if(!pdf_loc_conf){
    return NGX_CONF_ERROR;
  }

  pdf_loc_conf->enable = NGX_CONF_UNSET;
  return pdf_loc_conf;
}


static char * ngx_http_pdf_merge_loc_conf(ngx_conf_t *ngx_conf, void *parent, void *child)
{
  //ngx_http_pdf_loc_conf_t *prev = parent;
  //ngx_http_pdf_loc_conf_t *conf = child;
  //TODO: init pdf instance?
  return NGX_CONF_OK;
}


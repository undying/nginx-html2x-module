
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_thread_pool.h>

#include "ngx_http_pdf_wkhtmltopdf.h"


typedef struct {
  ngx_flag_t enable;
  pdf_global_conf_t *pdf_global_conf;
  ngx_thread_pool_t *thread_pool;
} ngx_http_pdf_loc_conf_t;

static const char ngx_http_pdf_content_type[] = "application/pdf";

static char * ngx_http_pdf(ngx_conf_t *ngx_conf, ngx_command_t *ngx_command, void *conf);

static ngx_int_t ngx_http_pdf_preconf(ngx_conf_t *ngx_conf);
static ngx_int_t ngx_http_pdf_postconf(ngx_conf_t *ngx_conf);
static ngx_int_t ngx_http_pdf_init(ngx_cycle_t *ngx_cycle);

static void ngx_http_pdf_deinit(ngx_cycle_t *ngx_cycle);

static void * ngx_http_pdf_create_loc_conf(ngx_conf_t *ngx_conf);
static char * ngx_http_pdf_merge_loc_conf(ngx_conf_t *ngx_conf, void *parent, void *child);

static ngx_int_t ngx_http_pdf_handler(ngx_http_request_t *r);
static void ngx_http_pdf_request_body(ngx_http_request_t *r);


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
  NULL,	                         /* init module */
  ngx_http_pdf_init,	           /* init process */
  NULL,                          /* init thread */
  NULL,                          /* exit thread */
  ngx_http_pdf_deinit,	         /* exit process */
  NULL,	                         /* exit master */
  NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_pdf_init(ngx_cycle_t *ngx_cycle)
{
  pdf_init();
  return NGX_OK;
}


static void
ngx_http_pdf_deinit(ngx_cycle_t *ngx_cycle)
{
  pdf_deinit();
}


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


static void ngx_http_pdf_request_body(ngx_http_request_t *r)
{
  int rc;
  ngx_buf_t *b;
  ngx_chain_t *in, out;
  unsigned char *bb = NULL;

  ngx_http_pdf_loc_conf_t *pdf_loc_conf;
  pdf_conf_t pdf_conf;

  if(!r->request_body){
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
        "HTML body not passed");
    ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
    return;
  }

  pdf_loc_conf = ngx_http_get_module_loc_conf(r, ngx_http_pdf_module);
  pdf_conf_init(pdf_loc_conf->pdf_global_conf, &pdf_conf);

  for(in = r->request_body->bufs; in; in = in->next){
    if(ngx_buf_in_memory(in->buf)){
      rc = (in->buf->last - in->buf->pos) + 1;
      bb = ngx_palloc(r->pool, rc);
      ngx_cpystrn(bb, in->buf->pos, rc);

      pdf_object_add(&pdf_conf, (char *)bb);
    } else if(in->buf->in_file){
      rc = in->buf->file_last + 1;
      bb = ngx_palloc(r->pool, rc);

      ngx_read_file(in->buf->file, bb, in->buf->file_last, in->buf->file_pos);
      bb[in->buf->file_last] = '\0';

      pdf_object_add(&pdf_conf, (char *)bb);
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

  pdf_conf_deinit(&pdf_conf);
  ngx_http_finalize_request(r, rc);
}


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


static void * ngx_http_pdf_create_loc_conf(ngx_conf_t *ngx_conf)
{
  ngx_http_pdf_loc_conf_t *pdf_loc_conf;

  pdf_loc_conf = ngx_pcalloc(ngx_conf->pool, sizeof(ngx_http_pdf_loc_conf_t));
  if(!pdf_loc_conf){
    return NGX_CONF_ERROR;
  }
  pdf_loc_conf->pdf_global_conf = ngx_pcalloc(ngx_conf->pool, sizeof(pdf_global_conf_t));
  if(!pdf_loc_conf->pdf_global_conf){
    return NGX_CONF_ERROR;
  }

  pdf_global_conf_init(pdf_loc_conf->pdf_global_conf);
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


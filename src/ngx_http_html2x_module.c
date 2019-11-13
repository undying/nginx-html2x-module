
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_thread_pool.h>

#include "ngx_http_html2pdf_wkhtmltopdf.h"

typedef struct {
  ngx_flag_t enable;
  ngx_uint_t html2x_dpi;
  ngx_uint_t html2x_image_dpi;
  ngx_uint_t html2x_header_fontsize;

  ngx_str_t html2x_web_default_encoding;
  ngx_str_t html2x_margin_top;
  ngx_str_t html2x_margin_right;
  ngx_str_t html2x_margin_left;
  ngx_str_t html2x_margin_bottom;
  ngx_str_t html2x_size_page_size;
  ngx_str_t html2x_header_center;

  ngx_array_t *html2x_load_custom_headers;

  html2pdf_global_conf_t *html2pdf_global_conf;
} ngx_http_html2x_loc_conf_t;


static const char ngx_http_pdf_content_type[] = "application/pdf";
static char * ngx_http_html2pdf(ngx_conf_t *ngx_conf, ngx_command_t *ngx_command, void *conf);

static ngx_int_t ngx_http_html2x_init_process(ngx_cycle_t *ngx_cycle);
static void ngx_http_html2x_exit_process(ngx_cycle_t *ngx_cycle);

static void * ngx_http_html2x_create_loc_conf(ngx_conf_t *ngx_conf);
static char * ngx_http_html2x_merge_loc_conf(ngx_conf_t *ngx_conf, void *parent, void *child);

static ngx_int_t ngx_http_html2pdf_handler(ngx_http_request_t *r);
static void ngx_http_pdf_request_body(ngx_http_request_t *r);


static ngx_command_t ngx_http_html2x_commands[] = {
  { 
    ngx_string("html2pdf"), 
    NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
    ngx_http_html2pdf,
    NGX_HTTP_LOC_CONF_OFFSET,
    0,
    NULL
  },
  {
    ngx_string("html2x_dpi"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_num_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, html2x_dpi),
    NULL
  },
  {
    ngx_string("html2x_image_dpi"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_num_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, html2x_image_dpi),
    NULL
  },
  {
    ngx_string("html2x_header_fontsize"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_num_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, html2x_header_fontsize),
    NULL
  },
  {
    ngx_string("html2x_web_default_encoding"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, html2x_web_default_encoding),
    NULL
  },
  {
    ngx_string("html2x_margin_top"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, html2x_margin_top),
    NULL
  },
  {
    ngx_string("html2x_margin_right"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, html2x_margin_right),
    NULL
  },
  {
    ngx_string("html2x_margin_bottom"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, html2x_margin_bottom),
    NULL
  },
  {
    ngx_string("html2x_margin_left"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, html2x_margin_left),
    NULL
  },
  {
    ngx_string("html2x_size_page_size"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, html2x_size_page_size),
    NULL
  },
  {
    ngx_string("html2x_header_center"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, html2x_header_center),
    NULL
  },
  {
    ngx_string("html2x_load_custom_headers"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE2,
    ngx_conf_set_keyval_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, html2x_load_custom_headers),
    NULL
  },
  ngx_null_command
};

static ngx_http_module_t ngx_http_html2x_module_ctx = {
  NULL, /* preconfiguration */
  NULL, /* postconfiguration */

  NULL, /* create main configuration */
  NULL, /* init main configuration */

  NULL, /* create server configuration */
  NULL, /* merge server configuration */

  ngx_http_html2x_create_loc_conf, /* create location configuration */
  ngx_http_html2x_merge_loc_conf /* merge location configuration */
};

ngx_module_t ngx_http_html2x_module = {
  NGX_MODULE_V1,
  &ngx_http_html2x_module_ctx,    /* module context */
  ngx_http_html2x_commands,	      /* module directives */
  NGX_HTTP_MODULE,	              /* module type */
  NULL,	                          /* init master */
  NULL,	                          /* init module */
  ngx_http_html2x_init_process,	  /* init process */
  NULL,	                          /* init thread */
  NULL,	                          /* exit thread */
  ngx_http_html2x_exit_process,	  /* exit process */
  NULL,	                          /* exit master */
  NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_html2x_init_process(ngx_cycle_t *ngx_cycle)
{
  pdf_init();
  return NGX_OK;
}


static void
ngx_http_html2x_exit_process(ngx_cycle_t *ngx_cycle)
{
  pdf_deinit();
}


static ngx_int_t
ngx_http_html2pdf_handler(ngx_http_request_t *r)
{
  ngx_int_t rc;

  if(r->method & (NGX_HTTP_HEAD|NGX_HTTP_GET)){
    return NGX_HTTP_NOT_ALLOWED;
  }

  rc = ngx_http_read_client_request_body(r, ngx_http_pdf_request_body);
  if(rc >= NGX_HTTP_SPECIAL_RESPONSE){
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
        "Unable to read request body");
    return rc;
  }

  return NGX_DONE;
}


static void
ngx_http_pdf_request_body(ngx_http_request_t *r)
{
  int rc;
  ngx_buf_t *b;
  ngx_chain_t *in, out;
  unsigned char *bb = NULL;

  ngx_http_html2x_loc_conf_t *html2x_loc_conf;
  html2pdf_conf_t html2pdf_conf;

  if(!r->request_body){
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
        "HTML body not passed");
    ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
    return;
  }

  html2x_loc_conf = ngx_http_get_module_loc_conf(r, ngx_http_html2x_module);
  pdf_conf_init(html2x_loc_conf->html2pdf_global_conf, &html2pdf_conf);

  for(in = r->request_body->bufs; in; in = in->next){
    if(ngx_buf_in_memory(in->buf)){
      rc = (in->buf->last - in->buf->pos) + 1;
      bb = ngx_palloc(r->pool, rc);
      if(!bb){
        goto alloc_error;
      }

      ngx_cpystrn(bb, in->buf->pos, rc);
    } else if(in->buf->in_file){
      rc = in->buf->file_last + 1;
      bb = ngx_palloc(r->pool, rc);
      if(!bb){
        goto alloc_error;
      }

      ngx_read_file(in->buf->file, bb, in->buf->file_last, in->buf->file_pos);
      bb[in->buf->file_last] = '\0';
    }

    pdf_object_add(&html2pdf_conf, (char *)bb);
  }

  b = ngx_calloc_buf(r->pool);
  if(!b){
    goto alloc_error;
  }

  rc = pdf_convert(&html2pdf_conf, &b->pos);
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
  ngx_http_finalize_request(r, rc);
  pdf_conf_deinit(&html2pdf_conf);

  return;

alloc_error:
  ngx_log_error(NGX_LOG_CRIT, r->connection->log, 0,
      "Unable to allocate buffer");
  ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
}


static char *
ngx_http_html2pdf(ngx_conf_t *ngx_conf, ngx_command_t *ngx_command, void *conf){
  ngx_http_core_loc_conf_t *ngx_core_loc_conf;
  ngx_http_html2x_loc_conf_t *html2x_loc_conf = conf;

  ngx_core_loc_conf = ngx_http_conf_get_module_loc_conf(ngx_conf, ngx_http_core_module);
  ngx_core_loc_conf->handler = ngx_http_html2pdf_handler;

  html2x_loc_conf->enable = 1;

  return NGX_CONF_OK;
}


static void *
ngx_http_html2x_create_loc_conf(ngx_conf_t *ngx_conf)
{
  ngx_http_html2x_loc_conf_t *html2x_loc_conf;
  html2x_loc_conf = ngx_pcalloc(ngx_conf->pool, sizeof(ngx_http_html2x_loc_conf_t));
  if(!html2x_loc_conf){
    return NGX_CONF_ERROR;
  }

  html2x_loc_conf->html2pdf_global_conf = ngx_pcalloc(ngx_conf->pool, sizeof(html2pdf_global_conf_t));
  if(!html2x_loc_conf->html2pdf_global_conf){
    return NGX_CONF_ERROR;
  }

  pdf_global_conf_init(html2x_loc_conf->html2pdf_global_conf);

  html2x_loc_conf->enable = NGX_CONF_UNSET;

  html2x_loc_conf->html2x_dpi = NGX_CONF_UNSET_UINT;
  html2x_loc_conf->html2x_image_dpi = NGX_CONF_UNSET_UINT;
  html2x_loc_conf->html2x_header_fontsize = NGX_CONF_UNSET_UINT;

  html2x_loc_conf->html2x_load_custom_headers = NULL;

  return html2x_loc_conf;
}


static char *
ngx_http_html2x_merge_loc_conf(ngx_conf_t *ngx_conf, void *parent, void *child)
{
  //ngx_http_html2x_loc_conf_t *prev = parent;
  //ngx_http_html2x_loc_conf_t *conf = child;
  return NGX_CONF_OK;
}


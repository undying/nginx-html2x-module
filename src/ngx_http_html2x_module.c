
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "ngx_http_html2pdf_wkhtmltopdf.h"

#define IS_VARIABLE(str) (str[0] == '$')

typedef struct {
  ngx_flag_t enable;

  wkhtmltopdf_global_t wk_gs;
  wkhtmltopdf_object_t wk_os;
} ngx_http_html2x_loc_conf_t;


static const char ngx_http_pdf_content_type[] = "application/pdf";
static char * ngx_http_html2pdf(ngx_conf_t *ngx_conf, ngx_command_t *ngx_command, void *conf);

static ngx_int_t ngx_http_html2x_init_process(ngx_cycle_t *ngx_cycle);
static void ngx_http_html2x_exit_process(ngx_cycle_t *ngx_cycle);

static void * ngx_http_html2x_create_loc_conf(ngx_conf_t *ngx_conf);
static char * ngx_http_html2x_merge_loc_conf(ngx_conf_t *ngx_conf, void *parent, void *child);

static ngx_int_t ngx_http_html2pdf_handler(ngx_http_request_t *r);
static void ngx_http_html2pdf_request_body(ngx_http_request_t *r);

static void ngx_http_html2pdf_configure(ngx_http_request_t *r, ngx_http_html2x_loc_conf_t *html2x_loc_conf, h2p_wkhtmltopdf_conf_t *wkhtmltopdf_conf);

static ngx_http_variable_value_t * ngx_http_html2x_get_variable(ngx_http_request_t *r, ngx_str_t *name);
static unsigned char * ngx_http_html2x_variable_value_get(ngx_http_request_t *r, ngx_str_t *name);

static void ngx_http_html2pdf_wk_global_set(ngx_http_request_t *r, wkhtmltopdf_global_settings *wk_gs, char *name, ngx_str_t *value);
static void ngx_http_html2pdf_wk_object_set(ngx_http_request_t *r, wkhtmltopdf_object_settings *wk_os, char *name, ngx_str_t *value);


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
    ngx_string("wkhtmltopdf_dpi"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, wk_gs.wkhtmltopdf_dpi),
    NULL
  },
  {
    ngx_string("wkhtmltopdf_image_dpi"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, wk_gs.wkhtmltopdf_image_dpi),
    NULL
  },
  {
    ngx_string("wkhtmltopdf_header_fontsize"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, wk_os.wkhtmltopdf_header_fontsize),
    NULL
  },
  {
    ngx_string("wkhtmltopdf_web_default_encoding"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, wk_os.wkhtmltopdf_web_default_encoding),
    NULL
  },
  {
    ngx_string("wkhtmltopdf_margin_top"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, wk_gs.wkhtmltopdf_margin_top),
    NULL
  },
  {
    ngx_string("wkhtmltopdf_margin_right"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, wk_gs.wkhtmltopdf_margin_right),
    NULL
  },
  {
    ngx_string("wkhtmltopdf_margin_bottom"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, wk_gs.wkhtmltopdf_margin_bottom),
    NULL
  },
  {
    ngx_string("wkhtmltopdf_margin_left"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, wk_gs.wkhtmltopdf_margin_left),
    NULL
  },
  {
    ngx_string("wkhtmltopdf_size_page_size"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, wk_gs.wkhtmltopdf_size_page_size),
    NULL
  },
  {
    ngx_string("wkhtmltopdf_header_center"),
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, wk_os.wkhtmltopdf_header_center),
    NULL
  },
  {
    ngx_string("wkhtmltopdf_load_custom_headers"), // not implemented in C-API
    NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE2,
    ngx_conf_set_keyval_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_html2x_loc_conf_t, wk_os.wkhtmltopdf_load_custom_headers),
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
  h2p_wkhtmltopdf_init();
  return NGX_OK;
}


static void
ngx_http_html2x_exit_process(ngx_cycle_t *ngx_cycle)
{
  h2p_wkhtmltopdf_deinit();
}


static ngx_int_t
ngx_http_html2pdf_handler(ngx_http_request_t *r)
{
  ngx_int_t rc;

  if(r->method & (NGX_HTTP_HEAD|NGX_HTTP_GET)){
    return NGX_HTTP_NOT_ALLOWED;
  }

  rc = ngx_http_read_client_request_body(r, ngx_http_html2pdf_request_body);
  if(rc >= NGX_HTTP_SPECIAL_RESPONSE){
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
        "Unable to read request body");
    return rc;
  }

  return NGX_DONE;
}


static void
ngx_http_html2pdf_request_body(ngx_http_request_t *r)
{
  int rc;
  ngx_buf_t *b = NULL;
  ngx_chain_t *in, out = {0};
  unsigned char *bb = NULL;
  h2p_wkhtmltopdf_conf_t wkhtmltopdf_conf;

  if(!r->request_body){
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
        "HTML body not passed");
    goto error;
  }

  h2p_wkhtmltopdf_conf_init(&wkhtmltopdf_conf);

  ngx_http_html2x_loc_conf_t *html2x_loc_conf;
  html2x_loc_conf = ngx_http_get_module_loc_conf(r, ngx_http_html2x_module);

  ngx_http_html2pdf_configure(r, html2x_loc_conf, &wkhtmltopdf_conf);

  for(in = r->request_body->bufs; in; in = in->next){
    if(ngx_buf_in_memory(in->buf)){
      rc = (in->buf->last - in->buf->pos) + 1;
      bb = ngx_palloc(r->pool, rc);
      if(!bb) goto alloc_error;

      ngx_memcpy(bb, in->buf->pos, rc);
    } else if(in->buf->in_file){
      rc = in->buf->file_last + 1;
      bb = ngx_palloc(r->pool, rc);
      if(!bb) goto alloc_error;

      ngx_read_file(in->buf->file, bb, in->buf->file_last, in->buf->file_pos);
      bb[in->buf->file_last] = '\0';
    }

    h2p_wkhtmltopdf_object_add(&wkhtmltopdf_conf, (char *)bb);
  }

  rc = h2p_wkhtmltopdf_convert(&wkhtmltopdf_conf, &bb);
  if(rc < 1){
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
        "unable to convert HTML to PDF");
    goto error;
  }

  b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
  if(!b) goto alloc_error;

  b->pos = ngx_palloc(r->pool, rc);
  if(!b->pos) goto alloc_error;

  ngx_memcpy(b->pos, bb, rc);

  b->last = b->pos + rc + 1;
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
    goto error;
  }

  rc = ngx_http_output_filter(r, &out);
  ngx_http_finalize_request(r, rc);
  h2p_wkhtmltopdf_conf_deinit(&wkhtmltopdf_conf);

  return;

alloc_error:
  ngx_log_error(NGX_LOG_CRIT, r->connection->log, 0,
      "Unable to allocate buffer");

error:
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

  html2x_loc_conf->enable = NGX_CONF_UNSET;
  html2x_loc_conf->wk_os.wkhtmltopdf_load_custom_headers = NULL;

  return html2x_loc_conf;
}


static char *
ngx_http_html2x_merge_loc_conf(ngx_conf_t *ngx_conf, void *parent, void *child)
{
  //ngx_http_html2x_loc_conf_t *prev = parent;
  //ngx_http_html2x_loc_conf_t *conf = child;
  return NGX_CONF_OK;
}

static void
ngx_http_html2pdf_configure(ngx_http_request_t *r, ngx_http_html2x_loc_conf_t *html2x_loc_conf, h2p_wkhtmltopdf_conf_t *wkhtmltopdf_conf)
{
  wkhtmltopdf_global_t *gs = &html2x_loc_conf->wk_gs;
  wkhtmltopdf_object_t *os = &html2x_loc_conf->wk_os;

  wkhtmltopdf_global_settings *wk_gs = wkhtmltopdf_conf->wk_gs;
  wkhtmltopdf_object_settings *wk_os = wkhtmltopdf_conf->wk_os;

  /* global settings */
  if(gs->wkhtmltopdf_margin_top.len){
    ngx_http_html2pdf_wk_global_set(r, wk_gs, "margin.top", &gs->wkhtmltopdf_margin_top);
  }

  if(gs->wkhtmltopdf_margin_right.len){
    ngx_http_html2pdf_wk_global_set(r, wk_gs, "margin.right", &gs->wkhtmltopdf_margin_right);
  }

  if(gs->wkhtmltopdf_margin_left.len){
    ngx_http_html2pdf_wk_global_set(r, wk_gs, "margin.left", &gs->wkhtmltopdf_margin_left);
  }

  if(gs->wkhtmltopdf_margin_bottom.len){
    ngx_http_html2pdf_wk_global_set(r, wk_gs, "margin.bottom", &gs->wkhtmltopdf_margin_bottom);
  }

  if(gs->wkhtmltopdf_size_page_size.len){
    ngx_http_html2pdf_wk_global_set(r, wk_gs, "size.pageSize", &gs->wkhtmltopdf_size_page_size);
  }

  if(gs->wkhtmltopdf_dpi.len){
    ngx_http_html2pdf_wk_global_set(r, wk_gs, "dpi", &gs->wkhtmltopdf_dpi);
  }

  if(gs->wkhtmltopdf_image_dpi.len){
    ngx_http_html2pdf_wk_global_set(r, wk_gs, "imageDPI", &gs->wkhtmltopdf_image_dpi);
  }


  /* object settings */
  if(os->wkhtmltopdf_web_default_encoding.len){
    ngx_http_html2pdf_wk_object_set(r, wk_os, "defaultEncoding", &os->wkhtmltopdf_web_default_encoding);
  }

  if(os->wkhtmltopdf_header_center.len){
    ngx_http_html2pdf_wk_object_set(r, wk_os, "header.center", &os->wkhtmltopdf_header_center);
  }

  if(os->wkhtmltopdf_header_fontsize.len){
    ngx_http_html2pdf_wk_object_set(r, wk_os, "header.fontSize", &os->wkhtmltopdf_header_fontsize);
  }
}


static ngx_http_variable_value_t *
ngx_http_html2x_get_variable(ngx_http_request_t *r, ngx_str_t *name)
{
  ngx_str_t str = {0};
  ngx_uint_t key = 0;

  str.len = name->len - 1;
  str.data = name->data + 1;

  key = ngx_hash_strlow(str.data, str.data, str.len);
  return ngx_http_get_variable(r, &str, key);
}


static unsigned char *
ngx_http_html2x_variable_value_get(ngx_http_request_t *r, ngx_str_t *name)
{
  unsigned char *p = NULL;
  ngx_http_variable_value_t *var;
  var = ngx_http_html2x_get_variable(r, name);

  if(!var) return NULL;
  if(var->not_found) return NULL;

  p = ngx_palloc(r->pool, var->len + 1);
  ngx_cpystrn(p, var->data, var->len + 1);

  return p;
}


static void
ngx_http_html2pdf_wk_global_set(ngx_http_request_t *r, wkhtmltopdf_global_settings *wk_gs, char *name, ngx_str_t *value)
{
  unsigned char *p = NULL;

  if(!IS_VARIABLE(value->data)){
    wkhtmltopdf_set_global_setting(wk_gs, (const char *)name, (const char *)value->data);
    return;
  }

  p = ngx_http_html2x_variable_value_get(r, value);
  if(!p){
    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
        "variable %s not found", value->data);
    return;
  }

  wkhtmltopdf_set_global_setting(wk_gs, (const char *)name, (const char *)p);
}


static void
ngx_http_html2pdf_wk_object_set(ngx_http_request_t *r, wkhtmltopdf_object_settings *wk_os, char *name, ngx_str_t *value)
{
  unsigned char *p = NULL;

  if(!IS_VARIABLE(value->data)){
    wkhtmltopdf_set_object_setting(wk_os, (const char *)name, (const char *)value->data);
    return;
  }

  p = ngx_http_html2x_variable_value_get(r, value);
  if(!p){
    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
        "variable %s not found", value->data);
    return;
  }

  wkhtmltopdf_set_object_setting(wk_os, (const char *)name, (const char *)p);
}


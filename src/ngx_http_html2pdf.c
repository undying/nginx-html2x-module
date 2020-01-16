
#include "ngx_http_html2x.h"
#include "ngx_http_html2pdf.h"


static const char ngx_http_pdf_content_type[] = "application/pdf";

static void ngx_http_html2pdf_request_body(ngx_http_request_t *r);
static void ngx_http_html2pdf_configure(ngx_http_request_t *r, ngx_http_html2x_loc_conf_t *html2x_loc_conf, h2p_wkhtmltopdf_conf_t *wkhtmltopdf_conf);

static void ngx_http_html2pdf_wk_global_set(ngx_http_request_t *r, wkhtmltopdf_global_settings *wk_gs, char *name, ngx_str_t *value);
static void ngx_http_html2pdf_wk_object_set(ngx_http_request_t *r, wkhtmltopdf_object_settings *wk_os, char *name, ngx_str_t *value);


char *
ngx_http_html2pdf(ngx_conf_t *ngx_conf, ngx_command_t *ngx_command, void *conf){
  ngx_http_core_loc_conf_t *ngx_core_loc_conf;
  ngx_http_html2x_loc_conf_t *html2x_loc_conf = conf;

  ngx_core_loc_conf = ngx_http_conf_get_module_loc_conf(ngx_conf, ngx_http_core_module);
  ngx_core_loc_conf->handler = ngx_http_html2pdf_handler;

  html2x_loc_conf->enable = 1;

  return NGX_CONF_OK;
}


ngx_int_t
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
  int rc = 0, bytes_sum = 0;
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
      rc += (in->buf->last - in->buf->pos);
    } else if(in->buf->in_file){
      rc += in->buf->file_last;
    }
  }

  bb = ngx_palloc(r->pool, rc + 1);
  if(!bb) goto alloc_error;

  for(in = r->request_body->bufs; in; in = in->next){
    if(ngx_buf_in_memory(in->buf)){
      rc = (in->buf->last - in->buf->pos);
      ngx_memcpy(bb + bytes_sum, in->buf->pos, rc);
    } else if(in->buf->in_file){
      rc = in->buf->file_last + 1;
      ngx_read_file(in->buf->file, bb + bytes_sum, in->buf->file_last, in->buf->file_pos);
    }
    bytes_sum += rc;
  }

  bb[bytes_sum] = '\0';
  h2p_wkhtmltopdf_object_add(&wkhtmltopdf_conf, (char *)bb);

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
    ngx_http_html2pdf_wk_object_set(r, wk_os, "web.defaultEncoding", &os->wkhtmltopdf_web_default_encoding);
  }

  if(os->wkhtmltopdf_header_center.len){
    ngx_http_html2pdf_wk_object_set(r, wk_os, "header.center", &os->wkhtmltopdf_header_center);
  }

  if(os->wkhtmltopdf_header_fontsize.len){
    ngx_http_html2pdf_wk_object_set(r, wk_os, "header.fontSize", &os->wkhtmltopdf_header_fontsize);
  }
}


static void
ngx_http_html2pdf_wk_global_set(ngx_http_request_t *r, wkhtmltopdf_global_settings *wk_gs, char *name, ngx_str_t *value)
{
  unsigned char *p = NULL;
  ngx_str_t *s;

  if(!HTML2X_IS_NGX_VARIABLE(value->data)){
    wkhtmltopdf_set_global_setting(wk_gs, (const char *)name, (const char *)value->data);
    return;
  }

  s = ngx_http_html2x_variable_value_get(r, value);
  if(!s){
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
        "variable %s not found", value->data);
    return;
  }

  p = ngx_palloc(r->pool, s->len + 1);
  if(!p){
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
        "allocation failed", value->data);
    return;
  }

  ngx_cpystrn(p, s->data, s->len + 1);
  wkhtmltopdf_set_global_setting(wk_gs, (const char *)name, (const char *)p);
}


static void
ngx_http_html2pdf_wk_object_set(ngx_http_request_t *r, wkhtmltopdf_object_settings *wk_os, char *name, ngx_str_t *value)
{
  unsigned char *b, *d, *p;
  ngx_str_t *s;

  if(!HTML2X_IS_NGX_VARIABLE(value->data)){
    wkhtmltopdf_set_object_setting(wk_os, (const char *)name, (const char *)value->data);
    return;
  }

  s = ngx_http_html2x_variable_value_get(r, value);
  if(!s){
    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
        "variable %s not found", value->data);
    return;
  }

  p = ngx_palloc(r->pool, s->len + 1);
  if(!p){
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
        "allocation failed");
    return;
  }

  ngx_cpystrn(p, s->data, s->len + 1);

  if(ngx_http_html2x_is_ngx_uri_arg(value)){
    b = d = ngx_palloc(r->pool, s->len);
    ngx_unescape_uri(&d, &p, s->len, NGX_UNESCAPE_URI);
    p = b;
    p[d - b] = '\0';
  }

  wkhtmltopdf_set_object_setting(wk_os, (const char *)name, (const char *)p);
}



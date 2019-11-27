
#include "ngx_http_html2x.h"


static ngx_int_t ngx_http_html2x_init_process(ngx_cycle_t *ngx_cycle);
static void ngx_http_html2x_exit_process(ngx_cycle_t *ngx_cycle);

static void * ngx_http_html2x_create_loc_conf(ngx_conf_t *ngx_conf);
static char * ngx_http_html2x_merge_loc_conf(ngx_conf_t *ngx_conf, void *parent, void *child);


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


ngx_http_variable_value_t *
ngx_http_html2x_get_variable(ngx_http_request_t *r, ngx_str_t *name)
{
  ngx_str_t str = {0};
  ngx_uint_t key = 0;

  str.len = name->len - 1;
  str.data = name->data + 1;

  key = ngx_hash_strlow(str.data, str.data, str.len);
  return ngx_http_get_variable(r, &str, key);
}


unsigned char *
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


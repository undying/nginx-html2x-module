
#ifndef NGX_HTTP_PDF_WKHTMLTOPDF
#define NGX_HTTP_PDF_WKHTMLTOPDF

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include <wkhtmltox/pdf.h>

#define WKHTMLTOX_USE_GRAPHICS 0


typedef struct {
  wkhtmltopdf_global_settings *wk_gs;
  wkhtmltopdf_object_settings *wk_os;
  wkhtmltopdf_converter *wk_c;
} h2p_wkhtmltopdf_conf_t;

typedef struct {
  ngx_str_t wkhtmltopdf_margin_top;
  ngx_str_t wkhtmltopdf_margin_right;
  ngx_str_t wkhtmltopdf_margin_left;
  ngx_str_t wkhtmltopdf_margin_bottom;
  ngx_str_t wkhtmltopdf_size_page_size;

  ngx_str_t wkhtmltopdf_dpi;
  ngx_str_t wkhtmltopdf_image_dpi;
} wkhtmltopdf_global_t;

typedef struct {
  ngx_str_t wkhtmltopdf_web_default_encoding;
  ngx_str_t wkhtmltopdf_header_center;

  ngx_str_t wkhtmltopdf_header_fontsize;
  ngx_array_t *wkhtmltopdf_load_custom_headers;
} wkhtmltopdf_object_t;


void h2p_wkhtmltopdf_init(void);
void h2p_wkhtmltopdf_deinit(void);

void h2p_wkhtmltopdf_conf_init(h2p_wkhtmltopdf_conf_t *wkhtmltopdf_conf);
void h2p_wkhtmltopdf_conf_deinit(h2p_wkhtmltopdf_conf_t *wkhtmltopdf_conf);

void h2p_wkhtmltopdf_object_add(h2p_wkhtmltopdf_conf_t *wkhtmltopdf_conf, char *html);
int h2p_wkhtmltopdf_convert(h2p_wkhtmltopdf_conf_t *wkhtmltopdf_conf, unsigned char **pdf);

#endif



#include "ngx_http_html2pdf_wkhtmltopdf.h"
#include "stddef.h"


void h2p_wkhtmltopdf_init(void)
{
  wkhtmltopdf_init(WKHTMLTOX_USE_GRAPHICS);
}


void h2p_wkhtmltopdf_deinit(){
  wkhtmltopdf_deinit();
}


void h2p_wkhtmltopdf_conf_init(h2p_wkhtmltopdf_conf_t *wkhtmltopdf_conf)
{
  wkhtmltopdf_conf->wk_gs = wkhtmltopdf_create_global_settings();
  wkhtmltopdf_conf->wk_os = wkhtmltopdf_create_object_settings();
  wkhtmltopdf_conf->wk_c = wkhtmltopdf_create_converter(wkhtmltopdf_conf->wk_gs);
}


void h2p_wkhtmltopdf_conf_deinit(h2p_wkhtmltopdf_conf_t *wkhtmltopdf_conf){
  wkhtmltopdf_destroy_global_settings(wkhtmltopdf_conf->wk_gs);
  wkhtmltopdf_destroy_object_settings(wkhtmltopdf_conf->wk_os);
  wkhtmltopdf_destroy_converter(wkhtmltopdf_conf->wk_c);
}


void h2p_wkhtmltopdf_object_add(h2p_wkhtmltopdf_conf_t *wkhtmltopdf_conf, char *html)
{
  wkhtmltopdf_add_object(wkhtmltopdf_conf->wk_c, wkhtmltopdf_conf->wk_os, html);
}


int h2p_wkhtmltopdf_convert(h2p_wkhtmltopdf_conf_t *wkhtmltopdf_conf, unsigned char **pdf){
  int rc;

  rc = wkhtmltopdf_convert(wkhtmltopdf_conf->wk_c);
  if(!rc) return rc;

  return wkhtmltopdf_get_output(wkhtmltopdf_conf->wk_c, (const unsigned char **)pdf);
}



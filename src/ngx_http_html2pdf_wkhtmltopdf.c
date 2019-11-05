
#include "ngx_http_html2pdf_wkhtmltopdf.h"
#include "stddef.h"


void pdf_init(void)
{
  wkhtmltopdf_init(WKHTMLTOX_USE_GRAPHICS);
}


void pdf_deinit(){
  wkhtmltopdf_deinit();
}


void pdf_conf_init(html2pdf_global_conf_t *html2pdf_global_conf, html2pdf_conf_t *html2pdf_conf)
{
  html2pdf_conf->wk_os = wkhtmltopdf_create_object_settings();
  html2pdf_conf->wk_c = wkhtmltopdf_create_converter(html2pdf_global_conf->wk_gs);
}


void pdf_global_conf_init(html2pdf_global_conf_t *html2pdf_global_conf){
  html2pdf_global_conf->wk_gs = wkhtmltopdf_create_global_settings();
}


void pdf_conf_deinit(html2pdf_conf_t *html2pdf_conf){
  wkhtmltopdf_destroy_object_settings(html2pdf_conf->wk_os);
  wkhtmltopdf_destroy_converter(html2pdf_conf->wk_c);
}


void pdf_object_add(html2pdf_conf_t *html2pdf_conf, char *html)
{
  wkhtmltopdf_add_object(html2pdf_conf->wk_c, html2pdf_conf->wk_os, html);
}


int pdf_convert(html2pdf_conf_t *html2pdf_conf, unsigned char **pdf){
  int rc;

  rc = wkhtmltopdf_convert(html2pdf_conf->wk_c);
  if(!rc)
    return rc;

  rc = wkhtmltopdf_get_output(html2pdf_conf->wk_c, (const unsigned char **)pdf);
  return rc;
}



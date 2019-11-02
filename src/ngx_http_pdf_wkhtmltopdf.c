
#include "ngx_http_pdf_wkhtmltopdf.h"
#include "stddef.h"


void pdf_init(void)
{
  wkhtmltopdf_init(WKHTMLTOX_USE_GRAPHICS);
}


void pdf_deinit(){
  wkhtmltopdf_deinit();
}


void pdf_conf_init(pdf_global_conf_t *pdf_global_conf, pdf_conf_t *pdf_conf)
{
  pdf_conf->wk_os = wkhtmltopdf_create_object_settings();
  pdf_conf->wk_c = wkhtmltopdf_create_converter(pdf_global_conf->wk_gs);
}


void pdf_global_conf_init(pdf_global_conf_t *pdf_global_conf){
  pdf_global_conf->wk_gs = wkhtmltopdf_create_global_settings();
}


void pdf_conf_deinit(pdf_conf_t *pdf_conf){
  wkhtmltopdf_destroy_object_settings(pdf_conf->wk_os);
  wkhtmltopdf_destroy_converter(pdf_conf->wk_c);
}


void pdf_object_add(pdf_conf_t *pdf_conf, char *html)
{
  wkhtmltopdf_add_object(pdf_conf->wk_c, pdf_conf->wk_os, html);
}


int pdf_convert(pdf_conf_t *pdf_conf, unsigned char **pdf){
  int rc;
  const unsigned char *pdf_p = NULL;

  rc = wkhtmltopdf_convert(pdf_conf->wk_c);
  if(!rc)
    return rc;

  rc = wkhtmltopdf_get_output(pdf_conf->wk_c, &pdf_p);
  *pdf = (unsigned char *)pdf_p;

  return rc;
}



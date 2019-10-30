
#include "ngx_http_pdf_wkhtmltopdf.h"
#include "stddef.h"


void pdf_init(pdf_conf_t * pdf_conf)
{
  wkhtmltopdf_init(WKHTMLTOX_USE_GRAPHICS);

  pdf_conf->wk_gs = wkhtmltopdf_create_global_settings();
  pdf_conf->wk_os = wkhtmltopdf_create_object_settings();
  pdf_conf->wk_c = wkhtmltopdf_create_converter(pdf_conf->wk_gs);
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


void pdf_deinit(pdf_conf_t * pdf_conf){
  wkhtmltopdf_destroy_converter(pdf_conf->wk_c);
  wkhtmltopdf_deinit();
}


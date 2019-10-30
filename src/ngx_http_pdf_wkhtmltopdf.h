
#ifndef NGX_HTTP_PDF_WKHTMLTOPDF
#define NGX_HTTP_PDF_WKHTMLTOPDF

#include <wkhtmltox/pdf.h>

#define WKHTMLTOX_USE_GRAPHICS 0


typedef struct {
  wkhtmltopdf_global_settings *wk_gs;
  wkhtmltopdf_object_settings *wk_os;
  wkhtmltopdf_converter *wk_c;
} pdf_conf_t;


void pdf_init(pdf_conf_t * pdf_conf);
void pdf_deinit(pdf_conf_t * pdf_conf);

void pdf_object_add(pdf_conf_t *pdf_conf, char *html);
int pdf_convert(pdf_conf_t *pdf_conf, unsigned char **pdf);

#endif


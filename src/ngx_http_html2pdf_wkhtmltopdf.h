
#ifndef NGX_HTTP_PDF_WKHTMLTOPDF
#define NGX_HTTP_PDF_WKHTMLTOPDF

#include <wkhtmltox/pdf.h>

#define WKHTMLTOX_USE_GRAPHICS 0


typedef struct {
  wkhtmltopdf_object_settings *wk_os;
  wkhtmltopdf_converter *wk_c;
} html2pdf_conf_t;

typedef struct {
  wkhtmltopdf_global_settings *wk_gs;
} html2pdf_global_conf_t;


void pdf_init(void);
void pdf_deinit(void);

void pdf_conf_init(html2pdf_global_conf_t *html2pdf_global_conf, html2pdf_conf_t *html2pdf_conf);
void pdf_conf_deinit(html2pdf_conf_t *html2pdf_conf);
void pdf_global_conf_init(html2pdf_global_conf_t *html2pdf_conf);

void pdf_object_add(html2pdf_conf_t *html2pdf_conf, char *html);
int pdf_convert(html2pdf_conf_t *html2pdf_conf, unsigned char **pdf);

#endif


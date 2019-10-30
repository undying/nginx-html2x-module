
#include<stdio.h>
#include<stdlib.h>
#include<wkhtmltox/pdf.h>

void wk_error(wkhtmltopdf_converter *wk_c, const char * msg){
  fprintf(stderr, "Error: %s\n", msg);
}

void wk_warning(wkhtmltopdf_converter *wk_c, const char * msg){
  fprintf(stderr, "Warning: %s\n", msg);
}


int main(int argc, char **argv){
  long int size = 0;
  const unsigned char *pdf = NULL;
  char *html;

  wkhtmltopdf_global_settings *wk_gs;
  wkhtmltopdf_object_settings *wk_os;
  wkhtmltopdf_converter *wk_c;

  FILE *f = fopen("/opt/mod_pdf/html/nginx news.htm", "r");
  if(!f){
    fprintf(stderr, "Error: unable to open html\n");
    return 1;
  }

  fseek(f, 0, SEEK_END);
  size = ftell(f);
  rewind(f);

  html = malloc(size + 1);
  fread(html, 1, size, f);
  fclose(f);

  html[size] = 0;

  wkhtmltopdf_init(0);

  wk_gs = wkhtmltopdf_create_global_settings();
  /* wkhtmltopdf_set_global_setting(wk_gs, "out", "result.pdf"); */

  wk_os = wkhtmltopdf_create_object_settings();
  /* wkhtmltopdf_set_object_setting(wk_os, "page", "https://nginx.org/"); */

  wk_c = wkhtmltopdf_create_converter(wk_gs);

  wkhtmltopdf_set_error_callback(wk_c, wk_error);
  wkhtmltopdf_set_warning_callback(wk_c, wk_warning);

  /* wkhtmltopdf_add_object(wk_c, wk_os, NULL); */
  wkhtmltopdf_add_object(wk_c, wk_os, html);

  if(!wkhtmltopdf_convert(wk_c)){
    wk_error(wk_c, "Conversion Failed!");
    return 1;
  }

  size = wkhtmltopdf_get_output(wk_c, &pdf);
  /* printf("httpErrorCode: %d\n", wkhtmltopdf_http_error_code(wk_c)); */

  f = fopen("/tmp/nginx.pdf", "wb");
  fwrite(pdf, 1, size, f);
  fclose(f);

  wkhtmltopdf_destroy_converter(wk_c);
  wkhtmltopdf_deinit();

  free(html);

  return 0;
}


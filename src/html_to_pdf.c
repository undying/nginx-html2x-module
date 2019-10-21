
#include<stdio.h>
#include<wkhtmltox/pdf.h>

void wk_error(wkhtmltopdf_converter *wk_c, const char * msg){
  fprintf(stderr, "Error: %s\n", msg);
}

void wk_warning(wkhtmltopdf_converter *wk_c, const char * msg){
  fprintf(stderr, "Warning: %s\n", msg);
}

int main(int argc, char **argv){
  wkhtmltopdf_global_settings *wk_gs;
  wkhtmltopdf_object_settings *wk_os;
  wkhtmltopdf_converter *wk_c;
  
  wkhtmltopdf_init(0);

  wk_gs = wkhtmltopdf_create_global_settings();
  wkhtmltopdf_set_global_setting(wk_gs, "out", "result.pdf");

  wk_os = wkhtmltopdf_create_object_settings();
  wkhtmltopdf_set_object_setting(wk_os, "page", "https://nginx.org/");

  wk_c = wkhtmltopdf_create_converter(wk_gs);

  wkhtmltopdf_set_error_callback(wk_c, wk_error);
  wkhtmltopdf_set_warning_callback(wk_c, wk_warning);

  wkhtmltopdf_add_object(wk_c, wk_os, NULL);

  if(!wkhtmltopdf_convert(wk_c)){
    wk_error(wk_c, "Conversion Failed!");
    return 1;
  }

  printf("httpErrorCode: %d\n", wkhtmltopdf_http_error_code(wk_c));

  wkhtmltopdf_destroy_converter(wk_c);
  wkhtmltopdf_deinit();

  return 0;
}


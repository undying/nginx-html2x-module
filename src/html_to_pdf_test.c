
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <wkhtmltox/pdf.h>

const char html[] = "<html><body>Hello!</body></html>";
const int cycle_n = 100;

void init_full(void){
  wkhtmltopdf_global_settings *wk_gs;
  wkhtmltopdf_object_settings *wk_os;
  wkhtmltopdf_converter *wk_c;

  unsigned char *pdf = NULL;

  for(int i = 0;i < cycle_n;i++){
    wkhtmltopdf_init(0);

    wk_gs = wkhtmltopdf_create_global_settings();
    wk_os = wkhtmltopdf_create_object_settings();
    wk_c = wkhtmltopdf_create_converter(wk_gs);

    wkhtmltopdf_add_object(wk_c, wk_os, html);
    wkhtmltopdf_convert(wk_c);

    wkhtmltopdf_get_output(wk_c, (const unsigned char **)&pdf);

    //wkhtmltopdf_destroy_object_settings(wk_os);
    //wkhtmltopdf_destroy_global_settings(wk_gs);
    wkhtmltopdf_destroy_converter(wk_c);

    //wkhtmltopdf_deinit();
    //printf("wkhtmltopdf_deinit: %d\n", rc);
  }
}

void init_conver(void){
  wkhtmltopdf_init(0);

  wkhtmltopdf_global_settings *wk_gs;
  wkhtmltopdf_object_settings *wk_os;
  wkhtmltopdf_converter *wk_c;

  unsigned char *pdf = NULL;

  wk_gs = wkhtmltopdf_create_global_settings();
  wk_os = wkhtmltopdf_create_object_settings();

  for(int i = 0;i < cycle_n; i++){
    wk_c = wkhtmltopdf_create_converter(wk_gs);

    wkhtmltopdf_add_object(wk_c, wk_os, html);
    wkhtmltopdf_convert(wk_c);

    wkhtmltopdf_get_output(wk_c, (const unsigned char **)&pdf);
    wkhtmltopdf_destroy_converter(wk_c);
  }

  wkhtmltopdf_deinit();
}

void init_once(void){
  wkhtmltopdf_init(0);

  wkhtmltopdf_global_settings *wk_gs;
  wkhtmltopdf_object_settings *wk_os;
  wkhtmltopdf_converter *wk_c;

  unsigned char *pdf = NULL;

  wk_gs = wkhtmltopdf_create_global_settings();
  wk_os = wkhtmltopdf_create_object_settings();
  wk_c = wkhtmltopdf_create_converter(wk_gs);

  for(int i = 0;i < cycle_n; i++){
    printf("cycle: %d\n", i);
    wkhtmltopdf_add_object(wk_c, wk_os, html);
    wkhtmltopdf_convert(wk_c);
    wkhtmltopdf_get_output(wk_c, (const unsigned char **)&pdf);
  }

  wkhtmltopdf_destroy_converter(wk_c);
  wkhtmltopdf_deinit();
}


int main(int argc, char **argv){
  clock_t time_start;
  clock_t time_end;

  //time_start = clock();
  //init_full();
  //time_end = clock();
  //printf("init_full: %f\n", (time_end - time_start) / (double)CLOCKS_PER_SEC);

  time_start = clock();
  init_conver();
  time_end = clock();
  printf("init_conver: %f\n", (time_end - time_start) / (double)CLOCKS_PER_SEC);

  //time_start = clock();
  //init_once();
  //time_end = clock();
  //printf("init_once: %f\n", (time_end - time_start) / (double)CLOCKS_PER_SEC);

  return 0;
}



###
  - более явная обработка ошибок. нужны колбэки на ошибки, для отображения в логе и для ответа вопрошающим
  - wkhtmltox знатно течёт. нужно посмотреть, как можно время от времени пересоздавать wkhtmltox, возможно считать количество запросов/используемую память
  - конвертация блочит воркер. можно попробовать заюзать треды nginx 
    - переносим процесс генерации из воркера в тред
      пока не получится, ибо wk запускает еще свои потоки и наступает блокировка.
      можно попробовать пропатчить wk, чтобы он использовал пул потоков самого nginx.

  - выносим конвертор в параметры модуля. создаем конвертор в момент создания процесса (треда?) и переиспользем
  
  - можно попробовать пропатчить wkhtmltopdf, чтобы высвобождать буффер после удаления конвертора
  - пропатчить object_add, чтобы можно было передавать размер буффера, а не полагаться на '\0'

### добавляем регулируемые параметры ###
# Опции от Вани | Аналоги в либе
------------------------------
custom-header | object_settings.load.customHeaders
custom-header-propagation | NULL
dpi           | global_settings.dpi
encoding      | object_settings.web.defaultEncoding
header-center | object_settings.header.center
header-font-size | object_settings.header.fontSize
image-dpi     | global_settings.imageDPI
margin-top    | global_settings.margin.top
margin-right  | global_settings.margin.right
margin-bottom | global_settings.margin.bottom
margin-left   | global_settings.margin.left
page-size     | global_settings.size.pageSize

# По сложности имплементации, в конце самое сложное
dpi           | global_settings.dpi
image-dpi     | global_settings.imageDPI
header-font-size | object_settings.header.fontSize
encoding      | object_settings.web.defaultEncoding
margin-top    | global_settings.margin.top
margin-right  | global_settings.margin.right
margin-bottom | global_settings.margin.bottom
margin-left   | global_settings.margin.left
page-size     | global_settings.size.pageSize
header-center | object_settings.header.center
custom-header | object_settings.load.customHeaders



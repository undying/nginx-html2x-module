
# html2x Nginx module

**Designed to generate PDF from HTML**

*Example:*
```sh
curl \
  -o /tmp/hello.pdf \
  -vd "<html><body>Hello!</body></html>" \
  -H 'Content-Type: text/html' \
  127.0.0.1/html2pdf?dpi=150
```

### How To

#### Docker

If you have Docker, you can run `make`.
Docker will build and start container.
Service will be available as `http://127.0.0.1/html2pdf`

#### Module Compilation

##### Install Requirements

###### Build Tools

`apt-get install -y git wget build-essential`

###### Wkhtmltox Dependencies

```sh
apt-get install -y \
  fontconfig \
  libfreetype6 \
  libjpeg-turbo8 \
  libpng16-16 \
  libx11-6 \
  libxcb1 \
  libxext6 \
  libxrender1 \
  xfonts-75dpi \
  xfonts-base
```

###### Wkhtmltox Library

```sh
export CODENAME=$(awk -F'=' '/CODENAME/ {print $2;exit}' /etc/os-release)
wget https://github.com/wkhtmltopdf/wkhtmltopdf/releases/download/0.12.5/wkhtmltox_0.12.5-1.${CODENAME}_amd64.deb
dpkg -i wkhtmltox_0.12.5-1.${CODENAME}_amd64.deb
```

###### Build Nginx

```sh
export nginx_v=1.16.1
export html2x_v=1.0.0
export html2x_path=nginx-html2x-module-${html2x_v}
export CPU_COUNT=$(grep -c processor /proc/cpuinfo)

wget https://github.com/undying/nginx-html2x-module/archive/v${html2x_v}.tar.gz
wget https://nginx.org/download/nginx-${nginx_v}.tar.gz

tar -xf v${html2x_v}.tar.gz
tar -xf nginx-${nginx_v}.tar.gz

cd nginx-${nginx_v}

./configure \
  --with-ld-opt="-Wl,-rpath,/usr/local/lib" \
  --add-module="../${html2x_path}"

make -j${CPU_COUNT}
make install
```


###### Nginx Configuration Example

```sh
server {
  listen 80;

  location /html2pdf {
    html2pdf;

    wkhtmltopdf_dpi $arg_dpi;
    wkhtmltopdf_image_dpi $arg_image_dpi;

    wkhtmltopdf_web_default_encoding $arg_default_encoding;

    wkhtmltopdf_header_center $arg_header_center;
    wkhtmltopdf_header_fontsize $arg_header_fontsize;

    wkhtmltopdf_margin_top $arg_margin_top;
    wkhtmltopdf_margin_right $arg_margin_right;
    wkhtmltopdf_margin_bottom $arg_margin_bottom;
    wkhtmltopdf_margin_left $arg_margin_left;

    wkhtmltopdf_size_page_size $arg_page_size;
  }
}
```


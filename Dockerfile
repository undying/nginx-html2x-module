
FROM ubuntu:18.04
CMD [ "/sbin/init.sh" ]

ENV DEBIAN_FRONTEND=noninteractive
ENV mod_pdf_path=/opt/mod_pdf

ENV deps_build="\
  gdb valgrind \
  wget \
  build-essential \
  "
ENV deps_runtime="\
  ca-certificates \
  "

ENV deps_build_nginx="\
  libgeoip-dev \
  zlib1g-dev \
  libssl-dev \
  libpcre3-dev \
  "
ENV deps_runtime_nginx="\
  geoip-bin \
  zlib1g \
  "

ENV deps_runtime_wkhtmltopdf="\
  fontconfig \
  libfreetype6 \
  libjpeg-turbo8 \
  libpng16-16 \
  libx11-6 \
  libxcb1 \
  libxext6 \
  libxrender1 \
  xfonts-75dpi \
  xfonts-base \
  "

ENV nginx_v=1.16.1
ENV wkhtmltopdf_v=0.12.5 wkhtmltopdf_deb_v=0.12.5-1

RUN set -x \
  && apt-get update \
  && apt-get upgrade -y \
  && apt-get install -y --no-install-recommends \
    ${deps_build} \
    ${deps_runtime} \
    ${deps_build_nginx} \
    ${deps_runtime_nginx} \
    ${deps_runtime_wkhtmltopdf}

RUN set -x \
  && export CPU_COUNT=$(grep -c processor /proc/cpuinfo) \
  && export CODENAME=$(awk -F'=' '/CODENAME/ {print $2;exit}' /etc/os-release) \
  && cd /opt/ \
  && printf "\
    https://github.com/wkhtmltopdf/wkhtmltopdf/releases/download/${wkhtmltopdf_v}/wkhtmltox_${wkhtmltopdf_deb_v}.${CODENAME}_amd64.deb\n \
    https://nginx.org/download/nginx-${nginx_v}.tar.gz" \
    |xargs -L1 -P${CPU_COUNT} -I{} wget --quiet {} \
  && ls *.gz|xargs -L1 -P${CPU_COUNT} -I{} tar -xzf {} \
  && ls *.deb|xargs dpkg -i \
  && rm -rfv *.gz *.deb


RUN mkdir -p ${mod_pdf_path}
COPY config ${mod_pdf_path}/
COPY lib ${mod_pdf_path}/lib
COPY src ${mod_pdf_path}/src

RUN set -x \
  && echo "building nginx" \
  && export CPU_COUNT=$(grep -c processor /proc/cpuinfo) \
  && cd /opt/nginx-${nginx_v} \
  && ./configure \
    --with-debug \
    --with-ld-opt="-Wl,-rpath,/usr/local/lib" \
    --prefix=/etc/nginx \
    --sbin-path=/usr/sbin/nginx \
    --conf-path=/etc/nginx/nginx.conf \
    --error-log-path=/var/log/nginx/error.log \
    --http-log-path=/var/log/nginx/access.log \
    --pid-path=/var/run/nginx.pid \
    --lock-path=/var/run/nginx.lock \
    \
    --http-client-body-temp-path=/var/cache/nginx/client_temp \
    --http-fastcgi-temp-path=/var/cache/nginx/fastcgi_temp \
    --http-proxy-temp-path=/var/cache/nginx/proxy_temp \
    --http-uwsgi-temp-path=/var/cache/nginx/uwsgi_temp \
    --http-scgi-temp-path=/var/cache/nginx/scgi_temp \
    \
    --user=nginx --group=nginx \
    \
    --with-cc-opt="-O0 -g -ggdb" \
    \
    --with-threads \
    --with-pcre-jit \
    --with-file-aio \
    \
    --with-http_v2_module \
    --with-http_ssl_module \
    --with-http_geoip_module \
    --with-http_realip_module \
    --with-http_gunzip_module \
    --with-http_gzip_static_module \
    \
    --without-http_ssi_module \
    --without-http_scgi_module \
    --without-http_uwsgi_module \
    --without-http_mirror_module \
    --without-http_fastcgi_module \
    --without-http_memcached_module \
    \
    --add-module="${mod_pdf_path}" \
    \
    && make -j${CPU_COUNT} \
    && make install \
    \
    && useradd --user-group --system nginx \
    \
    && install -d -o nginx -g nginx /var/cache/nginx/ \
    && install -d -o nginx -g nginx /etc/nginx/ /var/www/ \
    && install -o nginx conf/mime.types /etc/nginx/ \
    && install -o nginx html/* /var/www/ \
    && nginx -V

COPY root/ /



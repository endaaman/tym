FROM ubuntu:21.04

RUN apt-get update
RUN apt-get install -y tzdata
RUN apt-get install -y \
  autoconf \
  build-essential \
  cmake \
  git \
  libgirepository1.0-dev \
  libglib2.0-dev \
  libgnutls28-dev \
  libgtk-3-dev \
  liblua5.3-dev \
  libpcre2-dev \
  libsystemd-dev \
  meson \
  ninja-build \
  valac \
  xvfb

RUN git clone https://gitlab.gnome.org/GNOME/vte.git \
    && cd vte \
    && meson . build -Dsixel=true \
    && cd build \
    && ninja \
    && ninja install

RUN mkdir -p /var/app
ADD . /var/app
WORKDIR /var/app

RUN autoreconf -fvi
RUN PKG_CONFIG_PATH=/usr/local/lib/pkgconfig ./configure
RUN make
RUN LD_PRELOAD=/usr/local/lib/x86_64-linux-gnu/libvte-2.91.so make check
CMD LD_PRELOAD=/usr/local/lib/x86_64-linux-gnu/libvte-2.91.so xvfb-run -a ./src/tym -u ./lua/e2e.lua

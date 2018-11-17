FROM ubuntu:18.04

RUN apt-get update
RUN apt-get install -y build-essential autoconf libgtk-3-dev libvte-2.91-dev liblua5.3-dev git

RUN mkdir -p /var/app
ADD . /var/app
WORKDIR /var/app

RUN autoreconf -fvi
RUN ./configure
CMD ["/usr/bin/make", "/usr/bin/make", "check"]

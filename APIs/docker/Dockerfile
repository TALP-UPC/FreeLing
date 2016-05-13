FROM ubuntu:14.04
MAINTAINER padro@cs.upc.edu

RUN locale-gen en_US.UTF-8
RUN apt-get update -q
RUN apt-get install -y build-essential libicu-dev \
            libboost-regex-dev libboost-system-dev \
            libboost-program-options-dev libboost-thread-dev zlib1g-dev\
            automake autoconf libtool wget

WORKDIR /tmp
RUN wget --progress=dot:giga https://github.com/TALP-UPC/FreeLing/releases/download/4.0/FreeLing-4.0.tar.gz
RUN tar xvzf FreeLing-4.0.tar.gz
RUN rm -rf FreeLing-4.0.tar.gz
WORKDIR /tmp/FreeLing-4.0
RUN autoreconf --install
RUN ./configure
RUN make
RUN make install
WORKDIR /root
RUN rm -rf /tmp/FreeLing-4.0

EXPOSE 50005
CMD echo 'Hello world' | analyze -f en.cfg | grep -c 'world world NN 1'

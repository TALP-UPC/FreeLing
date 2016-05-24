FROM ubuntu:14.04
MAINTAINER padro@cs.upc.edu

RUN locale-gen en_US.UTF-8 && \
    apt-get update -q && \
    apt-get install -y build-essential automake autoconf libtool wget \
                       libicu52 libboost-regex1.54.0 \
                       libboost-system1.54.0 libboost-program-options1.54.0 \
                       libboost-thread1.54.0 && \
    apt-get install -y libicu-dev libboost-regex-dev libboost-system-dev \
                       libboost-program-options-dev libboost-thread-dev \
                       zlib1g-dev &&\
    cd /tmp && \
    wget --progress=dot:giga https://github.com/TALP-UPC/FreeLing/releases/download/4.0/FreeLing-4.0.tar.gz && \
    tar xvzf FreeLing-4.0.tar.gz && \
    rm -rf FreeLing-4.0.tar.gz && \
    cd /tmp/FreeLing-4.0 && \
    autoreconf --install && \
    ./configure && \
    make && \
    make install && \
    rm -rf /tmp/FreeLing-4.0 && \
    apt-get --purge -y remove build-essential libicu-dev \
            libboost-regex-dev libboost-system-dev \
            libboost-program-options-dev libboost-thread-dev zlib1g-dev\
            automake autoconf libtool wget && \
    apt-get autoremove -y && \
    apt-get clean -y && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /root

EXPOSE 50005
CMD echo 'Hello world' | analyze -f en.cfg | grep -c 'world world NN 1'

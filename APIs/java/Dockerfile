FROM ubuntu:trusty
MAINTAINER Alexander Merkulov <sasha@merqlove.ru>

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get -qq update

# Language setup
RUN apt-get -qqy install language-pack-en-base && \
    update-locale LANG=en_US.UTF-8 && \
    echo "LANGUAGE=en_US.UTF-8" >> /etc/default/locale && \
    echo "LC_ALL=en_US.UTF-8" >> /etc/default/locale

# Java Deps
RUN apt-get install -qq -y software-properties-common curl && \
    apt-get -qqy install git build-essential --fix-missing && \
    add-apt-repository ppa:webupd8team/java && apt-get -qq update && \
    echo oracle-java8-installer shared/accepted-oracle-license-v1-1 select true | /usr/bin/debconf-set-selections && \
    apt-get install -qq -y oracle-java8-installer

# Freeling Deps
RUN locale-gen en_US.UTF-8 && \
    apt-get install -y automake autoconf libtool wget swig \
                       libicu52 libboost-regex1.54.0 \
                       libboost-system1.54.0 libboost-program-options1.54.0 \
                       libboost-thread1.54.0 libboost-filesystem1.54.0 && \
    apt-get install -y libicu-dev libboost-regex-dev libboost-system-dev \
                       libboost-program-options-dev libboost-thread-dev \
                       zlib1g-dev

# Install Freeling
RUN cd /tmp && \
    wget -q https://github.com/TALP-UPC/FreeLing/releases/download/4.0/freeling-4.0-trusty-amd64.deb && \
    dpkg -i freeling-4.0-trusty-amd64.deb

# BUILD JNI
ENV JAVADIR /usr/lib/jvm/java-8-oracle
ENV SWIGDIR /usr/share/swig2.0
ENV FREELINGDIR /usr
ENV FREELINGOUT /usr/local/lib

RUN cd / && \
    wget -q https://github.com/TALP-UPC/FreeLing/releases/download/4.0/freeling-4.0.tar.gz && \
    tar -xzf freeling-4.0.tar.gz &&\
    cd FreeLing-4.0/APIs/java && \
    make all

# Cleanup
RUN sudo rm -f /tmp/freeling-4.0-trusty-amd64.deb && \
    sudo rm -f /tmp/freeling-4.0.tar.gz && \
    sudo rm -rf /FreeLing
    sudo apt-get autoremove -y && \
    sudo apt-get clean -y && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

WORKDIR /

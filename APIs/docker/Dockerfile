FROM debian:stable
MAINTAINER padro@cs.upc.edu

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Berlin

RUN echo "******** Installing dependencies... please wait" && \
    apt-get update -qq && \
    apt-get install -qq -y build-essential wget locales cmake \
                           libicu-dev libboost-regex-dev libboost-system-dev \
                           libboost-program-options-dev libboost-thread-dev \
                           libboost-filesystem-dev zlib1g-dev \
                           swig python3-dev default-jdk && \
    ( (echo en_US.UTF-8 UTF-8 >> /var/lib/locales/supported.d/en && locale-gen) || \
      (sed -i 's/^# \(en_US.UTF-8\)/\1/' /etc/locale.gen && locale-gen) || \
      locale-gen en_US.UTF-8 )

RUN export FL_VERSION=4.2 && \
    cd /tmp && \
    wget --progress=dot:giga https://github.com/TALP-UPC/FreeLing/releases/download/4.2/FreeLing-4.2.tar.gz && \
    tar xvzf FreeLing-$FL_VERSION.tar.gz && \
    rm -rf FreeLing-$FL_VERSION.tar.gz && \
    cd /tmp/FreeLing-$FL_VERSION && \

    mkdir build && \
    cd build && \
    echo "******** Compiling FreeLing... please wait" && \
    cmake .. -DJAVA_API=ON -DPYTHON3_API=ON -Wno-dev && \
    make -j8 && \
    echo "******** Installing FreeLing... please wait" && \
    make -j8 install && \

#   Uncomment to remove unwanted languages data to save space && \
#   rm -rf /usr/local/share/freeling/ru && \
#   rm -rf /usr/local/share/freeling/cy && \
#   rm -rf /usr/local/share/freeling/pt && \
#   etc ....

    rm -rf /tmp/FreeLing-$FL_VERSION && \
    apt-get --purge -y remove build-essential libicu-dev \
            libboost-regex-dev libboost-system-dev \
            libboost-program-options-dev libboost-thread-dev \
	    libboost-filesystem-dev zlib1g-dev\
            cmake wget swig python3-dev && \
    apt-get clean -y && \
    rm -rf /var/lib/apt/lists/*         

WORKDIR /root

EXPOSE 50005
CMD echo 'Hello world' | analyze -f en.cfg | grep -c 'world world NN 1'

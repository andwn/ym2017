FROM debian:stretch

ARG MARSDEV_REVISION

RUN apt-get update --quiet=2
RUN apt-get install \
        --assume-yes \
        --no-install-recommends \
        --quiet=2 \
        build-essential \
        libtool \
        autoconf \
        automake \
        autopoint \
        gettext \
        wget \
        texinfo \
        libpng-dev \
        openjdk-8-jre-headless \
        git \
        libboost-all-dev

RUN git clone \
        --quiet \
        --branch ${MARSDEV_REVISION} \
        --depth 1 \
        https://github.com/andwn/marsdev.git
RUN cd /marsdev && make

WORKDIR /src

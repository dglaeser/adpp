FROM ubuntu:22.04 AS builder
ARG TAG

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get upgrade -y -o Dpkg::Options::="--force-confold" \
    && apt-get install --no-install-recommends --yes \
        build-essential \
        cmake \
        git \
        python3 \
        ca-certificates \
        libmpc-dev \
        libgmp-dev \
        flex \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*


RUN git clone --depth 100 git://gcc.gnu.org/git/gcc.git gcc-${TAG}
RUN cd gcc-${TAG} && git reset --hard ${TAG}
RUN cd gcc-${TAG} && mkdir build && mkdir install && cd build && \
    ../configure --prefix=/gcc-${TAG}/install --disable-multilib
RUN cd gcc-${TAG}/build && make
RUN cd gcc-${TAG}/build && make install

FROM debian:stable-slim AS gcc
ARG TAG

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        openssl \
        ca-certificates \
        git \
        cmake \
        binutils \
        make \
        libmpc-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
COPY --from=builder /gcc-${TAG}/install /gcc-${TAG}/

# make sure the compiled libstdc++ is found
ENV LD_LIBRARY_PATH=/gcc-${TAG}/lib64/
ENV C_COMPILER=/gcc-${TAG}/bin/gcc
ENV CXX_COMPILER=/gcc-${TAG}/bin/g++

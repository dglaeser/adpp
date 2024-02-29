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
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN git clone --depth 1 -b ${TAG} "https://github.com/llvm/llvm-project.git" llvm-${TAG}
RUN cd llvm-${TAG} && \
    cmake -S llvm -B build -G "Unix Makefiles" \
        -DCMAKE_BUILD_TYPE=Release \
        -DLLVM_ENABLE_PROJECTS=clang \
        -DCMAKE_INSTALL_PREFIX=./install
RUN cd llvm-${TAG} && cmake --build build
RUN cd llvm-${TAG} && cmake --install build

FROM debian:stable-slim AS clang
ARG TAG

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        openssl \
        ca-certificates \
        git \
        cmake \
        libstdc++-12-dev \
        binutils \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
COPY --from=builder /llvm-${TAG}/install /llvm-${TAG}/

ENV C_COMPILER=/llvm-${TAG}/bin/clang
ENV CXX_COMPILER=/llvm-${TAG}/bin/clang++

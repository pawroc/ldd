FROM ubuntu:20.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y build-essential lzop u-boot-tools \
    net-tools bison flex libssl-dev libncurses5-dev libncursesw5-dev unzip chrpath \
    xz-utils minicom wget git-core cmake

RUN mkdir -p /workspace
WORKDIR /workspace
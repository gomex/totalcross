FROM ubuntu:bionic

MAINTAINER Allan César "acmlira@gmail.com"

RUN apt-get update
RUN apt-get install -y cmake ninja-build libsdl2-dev libfontconfig1-dev
ENV BUILD_FOLDER /build

WORKDIR ${BUILD_FOLDER}
CMD ["/bin/bash", "-c", "make", "-j$(($(nproc) + 2))", "-f", "${BUILD_FOLDER}/Makefile"]
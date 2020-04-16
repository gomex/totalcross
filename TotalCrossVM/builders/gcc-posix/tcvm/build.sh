#!/bin/bash
BASEDIR=$(dirname $0)
WORKDIR=$(cd $BASEDIR; pwd)

# execute docker run
sudo docker run \
-v $WORKDIR:/build \
-v $WORKDIR/../../../deps/skia:/skia \
-v $WORKDIR/../../../src:/src \
-e SRCDIR=/../../../src \
-e LIBS="-L. -lskia -lstdc++ -lpthread -lfontconfig -lGL -lSDL2main -lSDL2" \
-t totalcross/cross-compile:bionic \
bash -c "make  -j$(($(nproc) + 2)) -f /build/Makefile"

# bash -c "make  -j$(($(nproc) + 2)) -f ${WORKDIR}/Makefile"
#!/usr/bin/env bash
set -ex

sudo apt-get install libleveldb-dev
sudo echo "prefix=@prefix@
           exec_prefix=${prefix}
           libdir=${exec_prefix}/lib
           includedir=${prefix}/include
           Name: leveldb
           Description: A fast key-value storage
           Version: @version@
           Libs: -L${libdir} -lleveldb" > /usr/local/lib/pkgconfig/libleveldb.pc

if has Linux $NODE_LABELS; then
    sudo ldconfig
fi
#!/bin/bash

set -e -o pipefail

# setup: binance-cxx-api
pushd ./binance-cxx-api
  mkdir -p build
  pushd ./ThirdParty/curl
    git submodule init
    git submodule update
  popd
  pushd ./ThirdParty/jsoncpp
    git submodule init
    git submodule update
  popd
  pushd ./ThirdParty/libwebsockets
    git submodule init
    git submodule update
  popd
  pushd ./ThirdParty/mbedtls
    git submodule init
    git submodule update
  popd
  pushd ./build
  cmake ..
  make clean && make

  local op=`uname -o | tr '[:upper:]' '[:lower:]'`
  local arch=`uname -p`
  ./libbinance-cxx-api.so /usr/lib/${arch}-`echo ${op} | awk -F'/' 'print $2'`-`echo ${op} | awk -F'/' 'print $1'`
  popd

  cp ./include /usr/include/binance-cxx-api
popd

cp -r plog/include /usr/include/plog

mkdir -p build

pushd ./build
  cmake ..
  make clean && make
  sudo cp ./TransactionManager /usr/local/bin
popd

sudo mkdir -p /etc/binance
sudo cp config.ini /etc/binance

exit 0

#!/bin/bash

set -e -o pipefail

# setup: binance-cxx-api
pushd ./binance-cxx-api
  rm -rf build
  mkdir -p build
  pushd ./build
    cmake ..
    make

    op=`uname -o | tr '[:upper:]' '[:lower:]'`
    arch=`uname -p`
    cp ./libbinance-cxx-api.so /usr/lib/${arch}-`echo ${op} | awk -F'/' '{print $2}'`-`echo ${op} | awk -F'/' '{print $1}'`
  popd

  rm -rf /usr/include/binance-cxx-api
  cp -r ./include /usr/include/binance-cxx-api
popd

rm -rf /usr/include/plog
cp -r plog/include/plog /usr/include

mkdir -p build

pushd ./build
  cmake ..
  make clean && make
  sudo cp ./TransactionManager /usr/local/bin
popd

sudo mkdir -p /etc/binance
sudo cp config.ini /etc/binance

exit 0

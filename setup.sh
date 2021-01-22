#!/bin/bash

set -e -o pipefail

mkdir -p build

pushd ./build
  cmake ..
  make clean && make
  sudo cp ./TransactionManager /usr/local/bin
popd

sudo mkdir -p /etc/binance
sudo cp config.ini /etc/binance

exit 0

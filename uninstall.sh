#!/bin/bash

op=`uname -o | tr '[:upper:]' '[:lower:]'`
arch=`uname -p`

sudo rm -rf /usr/include/binance-cxx-api
sudo rm -rf /usr/lib/${arch}-`echo ${op} | awk -F'/' '{print $2}'`-`echo ${op} | awk -F'/' '{print $1}'`/libbinance-cxx-api.so
sudo rm -rf /usr/include/plog
sudo rm -rf /usr/local/bin/TransactionManager
sudo rm -rf /etc/binance/config.ini

service=`systemctl show transaction-manager | grep FragmentPath | awk -F= '{print $2}'`

sudo systemctl stop transaction-manager
has_service=$?
sudo systemctl disable transaction-manager

if [[ ${has_service} -eq 0 ]]
then
  sudo rm -rf ${service}
else
  echo "service not found."
fi

sudo systemctl daemon-reload

exit 0

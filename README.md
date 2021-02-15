# TransactionManager

## Clone

```bash
$ git clone --recurse-submodules https://github.com/d0ughnut/TransactionManager.git
```

## Require Pkg

```
build-essential
cmake (>= 3.13.4)
pkg-config
libglib2.0-dev
libjsoncpp-dev
libcurl4-gnutls-dev
```

## Setup

```bash
$ cd TransactionManager
$ ./setup.sh
```

## Configure

* configure `/etc/binance/config.ini`.

## Run

```bash
$ TransactionManager

# ex) 初期状態を設定する場合
# State: PURCHASE (即時購入)
#        SELL     (即時売却)
#        READY_P  (購入待機)
#        READY_S  (売却待機)
$ TransactionManager <State>
```

## Logic

1. cci が反発するのを待つ

  * 250, -250 のラインで反発した場合は即時決済する
  * 100, -100 のラインで反発した場合は macd シグナルが交差するまで待機する

2. macd が交差するのを待つ

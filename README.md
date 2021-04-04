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

|(t) cci \ macd        |macd >= signal|macd < signal
|:--                   |:--           |:--
|cci >= 0 && tcci >= 0 |買            |-
|cci >= 0 && tcci < 0  |-             |-
|cci < 0 && tcci < 0   |-             |売
|cci < 0 && tcci >= 0  |-             |-

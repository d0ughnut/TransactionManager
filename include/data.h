#pragma once

typedef struct _trade_data {
  double h_price;
  double l_price;
  double c_price;
} TradeData;

typedef struct _packet_data {
  double macd;
  double signal;
  double cci;
} PacketData;

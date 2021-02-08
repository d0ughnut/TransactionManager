#include "constant.h"

namespace ResponseTag {
  const int Kline::OPEN                       = 1;
  const int Kline::HIGH                       = 2;
  const int Kline::LOW                        = 3;
  const int Kline::CLOSE                      = 4;
  const int Kline::VOL                        = 5;
  const int Kline::CLOSE_TIME                 = 6;
  const int Kline::QUOTE_ASSET_VOL            = 7;
  const int Kline::NUM_OF_TRADES              = 8;
  const int Kline::TAKER_BUY_BASE_ASSET_VOL   = 9;
  const int Kline::TAKER_BUY_QUOTE_ASSET_VOL  = 10;
  const int Kline::IGNORE                     = 11;
}

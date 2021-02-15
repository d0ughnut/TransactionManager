#pragma once

namespace ResponseIdx {
class Kline {
  public:
    static const int OPEN_TIME;
    static const int OPEN;
    static const int HIGH;
    static const int LOW;
    static const int CLOSE;
    static const int VOL;
    static const int CLOSE_TIME;
    static const int QUOTE_ASSET_VOL;
    static const int NUM_OF_TRADES;
    static const int TAKER_BUY_BASE_ASSET_VOL;
    static const int TAKER_BUY_QUOTE_ASSET_VOL;
    static const int IGNORE;
};
}  // namespace ResponseIdx

namespace Property {
  enum class State {
    PURCHASE  = -2,
    READY_P   = -1,
    IDLE      = 0,
    READY_S   = 1,
    SELL      = 2
  };
}  // namespace Property

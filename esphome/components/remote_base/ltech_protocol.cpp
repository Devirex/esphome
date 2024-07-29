#include "ltech_protocol.h"
#include "esphome/core/log.h"

namespace esphome {
namespace remote_base {

static const char *const TAG = "remote.ltech";

static const int32_t TICK_US = 315;
static const int32_t HEADER_HIGH_US = 315;
static const int32_t HEADER_LOW_US = 1197;
static const int32_t BIT_ONE_HIGH_US = 630;
static const int32_t BIT_ONE_LOW_US = 283;
static const int32_t BIT_ZERO_HIGH_US = 315;
static const int32_t BIT_ZERO_LOW_US = 567;
static const int32_t FOOTER_MARK_US = 1197;

void LTECHProtocol::encode(RemoteTransmitData *dst, const LTECHData &data) {
  dst->set_carrier_frequency(38000);
  dst->reserve(2 + data.nbits * 2u);

  dst->item(HEADER_HIGH_US, HEADER_LOW_US);

  for (uint32_t mask = 1UL << (data.nbits - 1); mask != 0; mask >>= 1) {
    if (data.data & mask) {
      dst->item(BIT_ONE_HIGH_US, BIT_ONE_LOW_US);
    } else {
      dst->item(BIT_ZERO_HIGH_US, BIT_ZERO_LOW_US);
    }
  }

  dst->mark(BIT_ONE_HIGH_US);
}
optional<LTECHData> LTECHProtocol::decode(RemoteReceiveData src) {
  LTECHData out{
      .data = 0,
      .nbits = 0,
  };
  if (!src.expect_item(HEADER_HIGH_US,BIT_ZERO_HIGH_US)){
    return {};
  }
  ESP_LOGI(TAG, "Detected Header Start");
  while(!src.expect_space(HEADER_LOW_US)){
      if (!src.expect_item(HEADER_HIGH_US,BIT_ZERO_HIGH_US)){
          ESP_LOGI(TAG, "Wrong Format");
          return {};
      }
  }
  ESP_LOGI(TAG, "Detected Header Mark");
  
  for (out.nbits = 0; out.nbits < 208; out.nbits++) {
    if (src.expect_item(BIT_ONE_HIGH_US, BIT_ONE_LOW_US)) {
      out.data = (out.data << 1) | 1;
    } else if (src.expect_item(BIT_ZERO_HIGH_US, BIT_ZERO_LOW_US)) {
      out.data = (out.data << 1) | 0;
    } else if (out.nbits == 208) {
      return out;
    } else {
      return {};
    }
  }

  return out;
}
void LTECHProtocol::dump(const LTECHData &data) {
  ESP_LOGI(TAG, "Received LTECH: data=0x%08" PRIX32 ", nbits=%d", data.data, data.nbits);
}

}  // namespace remote_base
} 
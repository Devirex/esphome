#include "ltech_protocol.h"
#include "esphome/core/log.h"

namespace esphome {
namespace remote_base {

static const char *const TAG = "remote.ltech";

static const int32_t SYNC_US = 315;
static const int32_t HEADER_LOW_US = 1197;
static const int32_t BIT_ONE_HIGH_US = 630;
static const int32_t BIT_ONE_LOW_US = 283;
static const int32_t BIT_ZERO_HIGH_US = 315;
static const int32_t BIT_ZERO_LOW_US = 567;
static const int32_t FOOTER_MARK_US = 1197;

void LTECHProtocol::encode(RemoteTransmitData *dst, const LTECHData &data) {
  dst->set_carrier_frequency(38000);
  dst->reserve(2 + data.nbits * 2u);

  dst->item(SYNC_US, HEADER_LOW_US);

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
      .data[4] = 0,
      .nbits = 0,
  };
  while (src.expect_item(SYNC_US,SYNC_US)){
    ESP_LOGD(TAG, "Detected Sync");
  }

  if (!src.expect_item(SYNC_US, HEADER_LOW_US)) {
    return {};
  }
  ESP_LOGI(TAG, "Detected Header Mark");
  
  for (out.nbits = 0; out.nbits < 104; out.nbits++) {
    if (src.expect_item(BIT_ONE_HIGH_US, BIT_ONE_LOW_US)) {
      out.data[out.nbits/32] = (out.data[out.nbits/32] << 1) | 1;
    } else if (src.expect_item(BIT_ZERO_HIGH_US, BIT_ZERO_LOW_US)) {
      out.data[out.nbits/32] = (out.data[out.nbits/32] << 1) | 0;
    } else if (src.expect_mark(FOOTER_MARK_US)) {
      return out;
    } else {
      return {};
    }
  }

  return out;
}
void LTECHProtocol::dump(const LTECHData &data) {
  ESP_LOGI(TAG, "Received LTECH: 0x%08" PRIX32 "%08" PRIX32 "%08" PRIx32 "%08" PRIx32 ", nbits=%d", data.data[0], data.data[1], data.data[2] ,data.data[3], data.nbits );
}

}  // namespace remote_base
} 
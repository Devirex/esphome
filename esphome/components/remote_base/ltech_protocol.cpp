#include "ltech_protocol.h"
#include "esphome/core/log.h"

namespace esphome {
namespace remote_base {

static const char *const TAG = "remote.ltech";

static const int32_t TICK_US = 315;
static const int32_t HEADER_MARK_US = 1 * TICK_US;
static const int32_t HEADER_SPACE_US = -3.8 * TICK_US;
static const int32_t BIT_ONE_HIGH_US = 2 * TICK_US;
static const int32_t BIT_ONE_SPACE_US = -0.9 * TICK_US;
static const int32_t BIT_ZERO_HIGH_US = 1 * TICK_US;
static const int32_t BIT_ZERO_SPACE_US = -1.8 * TICK_US;
static const int32_t FOOTER_MARK_US = 3.8 * TICK_US;
static const int32_t FOOTER_SPACE_US = -51 * TICK_US;

void LTECHProtocol::encode(RemoteTransmitData *dst, const LGData &data) {
  dst->set_carrier_frequency(38000);
  dst->reserve(2 + data.nbits * 2u);

  dst->item(HEADER_HIGH_US, HEADER_LOW_US);

  for (uint32_t mask = 1UL << (data.nbits - 1); mask != 0; mask >>= 1) {
    if (data.data & mask) {
      dst->item(BIT_HIGH_US, BIT_ONE_LOW_US);
    } else {
      dst->item(BIT_HIGH_US, BIT_ZERO_LOW_US);
    }
  }

  dst->mark(BIT_HIGH_US);
}
optional<LGData> LTECHProtocol::decode(RemoteReceiveData src) {
  LGData out{
      .data = 0,
      .nbits = 0,
  };
  if (!src.expect_item(HEADER_MARK_US, HEADER_SPACE_US))
    return {};

  for (out.nbits = 0; out.nbits < 208; out.nbits++) {
    if (src.expect_item(BIT_ONE_HIGH_US, BIT_ONE_SPACE_US)) {
      out.data = (out.data << 1) | 1;
    } else if (src.expect_item(BIT_ZERO_HIGH_US, BIT_ZERO_SPACE_US)) {
      out.data = (out.data << 1) | 0;
    } else if (out.nbits == 28) {
      return out;
    } else {
      return {};
    }
  }

  return out;
}
void LTECHProtocol::dump(const LGData &data) {
  ESP_LOGI(TAG, "Received LG: data=0x%08" PRIX32 ", nbits=%d", data.data, data.nbits);
}

}  // namespace remote_base
} 
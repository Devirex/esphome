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
    if (data.address & mask) {
      dst->item(BIT_ONE_HIGH_US, BIT_ONE_LOW_US);
    } else {
      dst->item(BIT_ZERO_HIGH_US, BIT_ZERO_LOW_US);
    }
  }

  dst->mark(BIT_ONE_HIGH_US);
}
optional<LTECHData> LTECHProtocol::decode(RemoteReceiveData src) {
  LTECHData out{
      .address = 0,
      .mode = 0,
      .rgb = 0,
      .speed = 0,
      .white = 0,
      .function = 0,
      .crc = 0,
      .nbits = 0
  };

  while (src.expect_item(SYNC_US,SYNC_US))
    ESP_LOGVV(TAG, "Detected SYNC"); //Detect Sync

  if (!src.expect_item(SYNC_US, HEADER_LOW_US)) {
    return {};
  }
  ESP_LOGD(TAG, "Detected Header Mark");
  
  uint8_t byte = 0;
  for (out.nbits = 0; out.nbits < 104; out.nbits++) {
    if (src.expect_item(BIT_ONE_HIGH_US, BIT_ONE_LOW_US)) {
      byte = (byte << 1) | 1;
    } else if (src.expect_item(BIT_ZERO_HIGH_US, BIT_ZERO_LOW_US)) {
      byte = (byte << 1) | 0;
    } else if (src.expect_mark(FOOTER_MARK_US)) {
      ESP_LOGD(TAG, "Detected Footer Mark");
      return out;
    } else {
      return {};
    }

    if (out.nbits % 7 == 0) {
      byte = reverse_bits(byte);
      if (out.nbits < 32) {
      out.address = (out.address << 7) | byte;
      } else if (out.nbits < 40) {
        out.mode = (out.mode << 7) | byte;
      } else if (out.nbits < 64) {  
        out.rgb = (out.rgb << 7) | byte;
      } else if (out.nbits < 72) {
        out.function = (out.function << 7) | byte;
      } else if (out.nbits < 80) {
        out.white = (out.white << 7) | byte;
      } else if (out.nbits < 88) {
        out.speed = (out.speed << 7) | byte;
      } else if (out.nbits < 104) {
        out.crc = (out.crc << 7) | byte;
      }
    }
  }
  ESP_LOGD(TAG, "End of Data");
  return out;
}
void LTECHProtocol::dump(const LTECHData &data) {
  ESP_LOGI(TAG, "Received LTECH address: %08" PRIX32 ", mode: %02" PRIX32 ", rgb: %06" PRIX32 ", function: %02" PRIX32 ", white: %02" PRIX32 ", speed: %02" PRIX32 ", crc: %04" PRIX32 ", nbits=%d", data.address, data.mode, data.rgb, data.function, data.white, data.speed, data.crc, data.nbits );
} 
}  // namespace remote_base
} 
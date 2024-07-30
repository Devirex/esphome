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

uint16_t crc16_xmodem(std::initializer_list<uint8_t> data) {
    uint16_t crc = 0xFFFF; // Initialwert

    for (uint8_t byte : data) {
        crc ^= (byte << 8);
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ CRC16_XMODEM_POLY;
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;
}

void LTECHProtocol::encode(RemoteTransmitData *dst, const LTECHData &data) {
  dump(data);
  dst->set_carrier_frequency(38000);
  dst->reserve(2 + data.nbits * 2u);

  for (int8_t i = 0; i < 10; i++) {
    dst->item(SYNC_US, SYNC_US);
  }
  dst->item(SYNC_US, HEADER_LOW_US);

  for (uint8_t idx = 0 ; idx < data.nbits; idx++) {
    if (reinterpret_cast<const uint8_t *>(&data)[idx] == 1) {
      dst->item(BIT_ONE_HIGH_US, BIT_ONE_LOW_US);
    } else {
      dst->item(BIT_ZERO_HIGH_US, BIT_ZERO_LOW_US);
    }
  }
  dst->mark(FOOTER_MARK_US);
}

optional<LTECHData> LTECHProtocol::decode(RemoteReceiveData src) {
  LTECHData out{
      .address = 0,
      .data = 0,
      .check = 0,
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

    if (out.nbits % 8 == 7 && out.nbits > 0) {
      byte = reverse_bits(byte);
      if(out.nbits < 32) {
        out.address = (out.address << 8) | byte;
      }else if(out.nbits < 88) {
        out.data = (out.data << 8) | byte;
      }else if(out.nbits < 104) {
        out.check = (out.check << 8) | byte;
      }
    }


  }
  ESP_LOGD(TAG, "End of Data");
  return out;
}
void LTECHProtocol::dump(const LTECHData &data) {
  ESP_LOGI(TAG, "Received LTECH address: %08" PRIX32 ", data: %14" PRIX64 ", check: %04" PRIX32 ", nbits=%d", data.address, data.data, data.check, data.nbits );
} 
}  // namespace remote_base
} 
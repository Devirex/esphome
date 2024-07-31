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

uint16_t crc16_xmodem(const uint8_t* data, size_t length) {
    uint16_t crc = 0x0000; // Initialwert
    const uint16_t polynomial = 0x1021;

    for (size_t i = 0; i < length; ++i) {
        crc ^= (data[i] << 8); // Byte in das hohe Byte des CRC verschieben

        for (int j = 0; j < 8; ++j) { // Jedes Bit verarbeiten
            if (crc & 0x8000) { // Wenn das höchste Bit gesetzt ist
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

void LTECHProtocol::encode(RemoteTransmitData *dst, const LTECHData &data) {
  dump(data);
  dst->set_carrier_frequency(0);
  dst->reserve(2 + data.nbits * 2u);

  for (int8_t i = 0; i < 10; i++) {
    dst->item(SYNC_US, SYNC_US);
  }
  dst->item(SYNC_US, HEADER_LOW_US);
  sendBits(dst, data.address, 32);
  sendBits(dst, data.data, 56);
  sendBits(dst, data.check, 16);
  
  dst->mark(FOOTER_MARK_US);
  
}

void sendBits(RemoteTransmitData *dst, uint64_t data, int bitCount) {
    uint64_t reversedData = 0;
    for (int i = 0; i < bitCount; i++) {
        reversedData <<= 1;
        reversedData |= (data >> i) & 1;
    }
    reversedData = __bswap64(reversedData);
    for (int i = bitCount - 1; i >= 0; i--) {
        int bit = (reversedData >> i) & 1;
        if ((reversedData >> i) & 1) {
            dst->item(BIT_ONE_HIGH_US, BIT_ONE_LOW_US);
        } else {
            dst->item(BIT_ZERO_HIGH_US, BIT_ZERO_LOW_US);
        }
    }
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
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

uint8_t reverseBits(uint8_t byte) {
    uint8_t reversed = 0;
    for (int i = 0; i < 8; ++i) {
        if (byte & (1 << i)) {
            reversed |= (1 << (7 - i));
        }
    }
    return reversed;
}

// Generische Funktion zum Umkehren der Bits in 8-Bit-Schritten
template <typename T>
T rev8Bit(T value) {
    
    // Erzeuge einen neuen Wert, der die umgekehrten 8-Bit-Blöcke enthält
    T reversedValue = 0;
    int numBytes = sizeof(T);

    // Umkehre die Bits jedes einzelnen Bytes
    for (int i = 0; i < numBytes; ++i) {
        uint8_t currentByte = (value >> (i * 8)) & 0xFF;
        uint8_t reversedByte = reverseBits(currentByte);
        reversedValue |= (static_cast<T>(reversedByte) << (i * 8));
    }

    return reversedValue;
}


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
  
  for (out.nbits = 0; out.nbits < 104; out.nbits++) {
    if (src.expect_item(BIT_ONE_HIGH_US, BIT_ONE_LOW_US)) {
      if(out.nbits < 32) out.address = (out.address << 1) | 1UL;
      else if(out.nbits < 40) out.mode = (out.mode << 1) | 1UL;
      else if(out.nbits < 64) out.rgb = (out.rgb << 1) | 1UL;
      else if(out.nbits < 72) out.function = (out.function << 1) | 1UL;
      else if(out.nbits < 80) out.white = (out.white << 1) | 1UL;
      else if(out.nbits < 88) out.speed = (out.speed << 1) | 1UL;
      else out.crc = (out.crc << 1) | 1UL;
    } else if (src.expect_item(BIT_ZERO_HIGH_US, BIT_ZERO_LOW_US)) {
      if(out.nbits < 32) out.address = (out.address << 1) | 0UL;
      else if(out.nbits < 40) out.mode = (out.mode << 1) | 0UL;
      else if(out.nbits < 64) out.rgb = (out.rgb << 1) | 0UL;
      else if(out.nbits < 72) out.function = (out.function << 1) | 0UL;
      else if(out.nbits < 80) out.white = (out.white << 1) | 0UL;
      else if(out.nbits < 88) out.speed = (out.speed << 1) | 0UL;
      else out.crc = (out.crc << 1) | 0UL;
    } else if (src.expect_mark(FOOTER_MARK_US)) {
      ESP_LOGD(TAG, "Detected Footer Mark");
      return out;
    } else {
      return {};
    }
  }
  ESP_LOGD(TAG, "End of Data");
  return out;
}
void LTECHProtocol::dump(const LTECHData &data) {
  ESP_LOGI(TAG, "Received LTECH address: %08" PRIX32 ", mode: %02" PRIX32 ", rgb: %06" PRIX32 ", function: %02" PRIX32 ", white: %02" PRIX32 ", speed: %02" PRIX32 ", crc: %04" PRIX32 ", nbits=%d", rev8Bit(data.address), rev8Bit(data.mode), rev8Bit(data.rgb) , rev8Bit(data.function), rev8Bit(data.white), rev8Bit(data.speed), rev8Bit(data.crc), data.nbits );
} 
}  // namespace remote_base
} 
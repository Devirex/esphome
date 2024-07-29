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
  
  for (out.nbits = 0; out.nbits < 104; out.nbits++) {
    if (src.expect_item(BIT_ONE_HIGH_US, BIT_ONE_LOW_US)) {
      if(out.nbits < 32){
        out.address = (out.address << 1) | 1;
      }else if(out.nbits < 40){
        out.mode = (out.mode << 1) | 1;
      }else if(out.nbits < 64){
        out.rgb = (out.rgb << 1) | 1;
      }else if(out.nbits < 72){
        out.function = (out.function << 1) | 1;
      }else if(out.nbits < 80){
        out.white = (out.white << 1) | 1;
      }else if(out.nbits < 88){
        out.speed = (out.speed << 1) | 1;
      }else if(out.nbits < 104){
        out.crc = (out.crc << 1) | 1;
      }
    } else if (src.expect_item(BIT_ZERO_HIGH_US, BIT_ZERO_LOW_US)) {
      if(out.nbits < 32){
        out.address = (out.address << 1) | 0;
      }else if(out.nbits < 40){
        out.mode = (out.mode << 1)| 0;
      }else if(out.nbits < 64){
        out.rgb = (out.rgb << 1);
      }else if(out.nbits < 72){
        out.function = (out.function << 1)| 0;
      }else if(out.nbits < 80){
        out.white = (out.white << 1)| 0;
      }else if(out.nbits < 88){
        out.speed = (out.speed << 1)| 0;
      }else if(out.nbits < 104){
        out.crc = (out.crc << 1)| 0;
      }
    } else if (src.expect_mark(FOOTER_MARK_US)) {
      out.address = __builtin_bswap32(out.address);
      out.rgb = __builtin_bswap32(out.rgb);
      out.mode = __builtin_bswap32(out.mode);
      out.function = __builtin_bswap32(out.function);
      out.white = __builtin_bswap32(out.white);
      out.speed = __builtin_bswap32(out.speed);
      out.crc = __builtin_bswap16(out.crc);
      return out;
    } else {
      return {};
    }
  }

  return out;
}
void LTECHProtocol::dump(const LTECHData &data) {
  ESP_LOGI(TAG, "Received LTECH address: %08" PRIX32 ", mode: %02" PRIX32 ", rgb: %06" PRIX32 ", function: %02" PRIX32 ", white: %02" PRIX32 ", speed: %02" PRIX32 ", crc: %04" PRIX32 ", nbits=%d", data.address, data.mode, data.rgb , data.function, data.white, data.speed, data.crc, data.nbits );
} 
}  // namespace remote_base
} 
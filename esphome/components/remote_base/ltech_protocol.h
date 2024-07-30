#pragma once

#include "esphome/core/component.h"
#include "remote_base.h"

#include <cinttypes>

namespace esphome {
namespace remote_base {

struct LTECHData {
  uint32_t address;
  uint8_t mode;
  uint32_t rgb : 24;
  uint8_t speed;
  uint8_t white;
  uint8_t function;
  uint16_t crc;
  uint8_t nbits;

  bool operator==(const LTECHData &rhs) const { return address == rhs.address && mode == rhs.mode && rgb == rhs.rgb && function == rhs.function && white == rhs.white && speed == rhs.speed && crc == rhs.crc && nbits == rhs.nbits; }
  void calculate_crc(){ crc = crc16_xmodem({address, mode, rgb, function, white, speed}); }
};

class LTECHProtocol : public RemoteProtocol<LTECHData> {
 public:
  void encode(RemoteTransmitData *dst, const LTECHData &data) override;
  optional<LTECHData> decode(RemoteReceiveData src) override;
  void dump(const LTECHData &data) override;
};

DECLARE_REMOTE_PROTOCOL(LTECH)

template<typename... Ts> class LTECHAction : public RemoteTransmitterActionBase<Ts...> {
 public:
  TEMPLATABLE_VALUE(uint32_t, address)
  TEMPLATABLE_VALUE(uint8_t, mode)
  TEMPLATABLE_VALUE(uint32_t, rgb)
  TEMPLATABLE_VALUE(uint8_t, function)
  TEMPLATABLE_VALUE(uint8_t, white)
  TEMPLATABLE_VALUE(uint8_t, speed)
  TEMPLATABLE_VALUE(uint16_t, crc)
  TEMPLATABLE_VALUE(uint8_t, nbits)

  void encode(RemoteTransmitData *dst, Ts... x) override {
    LTECHData data{};
    data.address = this->address_.value(x...);
    data.mode = this->mode_.value(x...);
    data.rgb = this->rgb_.value(x...);
    data.function = this->function_.value(x...);
    data.white = this->white_.value(x...);
    data.speed = this->speed_.value(x...);
    data.crc = this->crc_.value(x...);
    data.nbits = this->nbits_.value(x...);
    LTECHProtocol().encode(dst, data);
  }
};

}  // namespace remote_base
}  // namespace esphome

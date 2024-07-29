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
  uint8_t function;
  uint8_t white;
  uint8_t speed;
  uint8_t nbits;
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
  TEMPLATABLE_VALUE(uint8_t, nbits)

  void encode(RemoteTransmitData *dst, Ts... x) override {
    LTECHData data{};
    data.data = this->data_.value(x...);
    data.nbits = this->nbits_.value(x...);
    LTECHProtocol().encode(dst, data);
  }
};

}  // namespace remote_base
}  // namespace esphome

#pragma once

#include "esphome/core/component.h"
#include "remote_base.h"

#include <cinttypes>

namespace esphome {
namespace remote_base {

struct LTECHData {
  uint32_t data;
  uint8_t nbits;

  bool operator==(const LTECHData &rhs) const { return data == rhs.data && nbits == rhs.nbits; }
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
  TEMPLATABLE_VALUE(uint32_t, data)
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

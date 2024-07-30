#pragma once

#include "esphome/core/component.h"
#include "remote_base.h"

#include <cinttypes>

namespace esphome {
namespace remote_base {


uint16_t crc16_xmodem(uint64_t data);
struct LTECHData {
  uint32_t address;
  uint64_t data : 56;
  uint16_t check;
  uint8_t nbits;

  bool operator==(const LTECHData &rhs) const { return address == rhs.address && data == rhs.data && check == rhs.check && nbits == rhs.nbits; }
};



class LTECHProtocol : public RemoteProtocol<LTECHData> {
 public:
  void encode(RemoteTransmitData *dst, const LTECHData &data) override;
  optional<LTECHData> decode(RemoteReceiveData src) override;
  void dump(const LTECHData &data) override;
 private:
  
};

DECLARE_REMOTE_PROTOCOL(LTECH)

template<typename... Ts> class LTECHAction : public RemoteTransmitterActionBase<Ts...> {
 public:
  TEMPLATABLE_VALUE(uint32_t, id)
  TEMPLATABLE_VALUE(uint64_t, data)
  TEMPLATABLE_VALUE(uint32_t, check)
  TEMPLATABLE_VALUE(uint8_t, nbits)

  void encode(RemoteTransmitData *dst, Ts... x) override {
    LTECHData data{};
    data.address = this->address_.value(x...);
    data.data = this->data_.value(x...);
    data.check = crc16_xmodem(data.data);
    data.nbits = this->nbits_.value(x...);
    LTECHProtocol().encode(dst, data);
  }
};



}  // namespace remote_base
}  // namespace esphome

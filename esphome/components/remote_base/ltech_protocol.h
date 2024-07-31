#pragma once

#include "esphome/core/component.h"
#include "remote_base.h"

#include <cinttypes>

namespace esphome {
namespace remote_base {

uint16_t crc16_xmodem(const uint8_t *data, size_t length);
void sendBits(RemoteTransmitData *dst, uint64_t data, int bitCount);
struct LTECHData {
  uint32_t address;
  uint64_t data : 56;
  uint16_t check;
  uint8_t nbits;

  bool operator==(const LTECHData &rhs) const { return address == rhs.address && data == rhs.data && check == rhs.check && nbits == rhs.nbits; }
  uint16_t calculate_crc() const {
        uint8_t byteArray[11]; // 4 Byte Address + 7 Byte Data
        for (int i = 0; i < 4; ++i) {
            byteArray[i] = static_cast<uint8_t>(address >> (24 - i * 8));
        }

        // Data in Byte-Array umwandeln (7 Byte)
        for (int i = 0; i < 7; ++i) {
            byteArray[4 + i] = static_cast<uint8_t>(data >> (48 - i * 8));
        }
        return crc16_xmodem(byteArray, sizeof(byteArray));
    }

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
  TEMPLATABLE_VALUE(uint32_t, address)
  TEMPLATABLE_VALUE(uint64_t, data)
  TEMPLATABLE_VALUE(uint32_t, check)
  TEMPLATABLE_VALUE(uint8_t, nbits)

  void encode(RemoteTransmitData *dst, Ts... x) override {
    LTECHData data{};
    data.address = this->address_.value(x...);
    data.data = this->data_.value(x...);
    data.check = data.calculate_crc();
    data.nbits = this->nbits_.value(x...);
    LTECHProtocol().encode(dst, data);
  }
};



}  // namespace remote_base
}  // namespace esphome

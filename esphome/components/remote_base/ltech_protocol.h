#pragma once

#include "esphome/core/component.h"
#include "remote_base.h"

#include <cinttypes>

namespace esphome {
namespace remote_base {


uint16_t crc16_xmodem(const uint8_t* data, size_t length);
struct LTECHData {
  uint32_t address;
  uint64_t data : 56;
  uint16_t check;
  uint8_t nbits;

  bool operator==(const LTECHData &rhs) const { return address == rhs.address && data == rhs.data && check == rhs.check && nbits == rhs.nbits; }
  uint16_t calculate_crc() const {
        // Byte-Array zum Packen der relevanten Daten (address und data)
        uint8_t buffer[8]; // address (4 Byte) + data (4 Byte), data wird auf 4 Byte gekürzt

        // address in die ersten 4 Bytes packen
        std::memcpy(buffer, &address, sizeof(address));
        
        // data in die nächsten 4 Bytes packen (obwohl data 7 Byte groß ist, nehmen wir nur die unteren 4 Byte)
        uint32_t truncatedData = static_cast<uint32_t>(data & 0xFFFFFFFFFFFF); // 56 Bit auf 64 Bit Maskierung
        std::memcpy(buffer + sizeof(address), &truncatedData, sizeof(truncatedData));

        // CRC16-Xmodem Berechnung
        return crc16_xmodem(buffer, sizeof(buffer));
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

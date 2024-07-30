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
        std::vector<uint8_t> crc_data;

        // Füge address hinzu
        crc_data.push_back(address & 0xFF);
        crc_data.push_back((address >> 8) & 0xFF);
        crc_data.push_back((address >> 16) & 0xFF);
        crc_data.push_back((address >> 24) & 0xFF);

        // Füge data hinzu (nur die unteren 56 Bits)
        uint64_t temp_data = data; // Erstelle eine Kopie des Datenfeldes
        for (size_t i = 0; i < 7; ++i) {
            crc_data.push_back(temp_data & 0xFF);
            temp_data >>= 8;
        }

        // CRC16-Xmodem Polynom ist 0x1021
        return crc16_xmodem(crc_data.data(), crc_data.size());
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

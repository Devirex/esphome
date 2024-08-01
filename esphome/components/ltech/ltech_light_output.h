#pragma once

#include "esphome/core/component.h"
#include "esphome/components/light/light_output.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ltech {

class LTECHLightOutput : public light::LightOutput {
 public:
  void set_address(uint32_t address) { address_ = address; }
  void set_color_interlock(bool color_interlock) { color_interlock_ = color_interlock; }
  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    if (this->color_interlock_)
      traits.set_supported_color_modes({light::ColorMode::RGB, light::ColorMode::WHITE});
    else
      traits.set_supported_color_modes({light::ColorMode::RGB_WHITE});
    return traits;
  }
  void write_state(light::LightState *state) override {
    //this->red_->set_level(red);
    //this->green_->set_level(green);
    //this->blue_->set_level(blue);
    //this->white_->set_level(white);
  }

  void update_state(light::LightState *state) override {
    float red, green, blue, white;
    state->current_values_as_rgbw(&red, &green, &blue, &white, this->color_interlock_);
    ESP_LOGVV("LTECH", "RGBW: %.2f, %.2f, %.2f, %.2f", red, green, blue, white);
    //this->red_->set_level(red);
    //this->green_->set_level(green);
    //this->blue_->set_level(blue);
    //this->white_->set_level(white);
  }

 protected:
  uint32_t address_;
  bool color_interlock_{false};
};

}  // namespace rgbw
}  // namespace esphome

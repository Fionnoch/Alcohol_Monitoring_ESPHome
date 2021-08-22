#pragma once

#include "esphome/core/component.h"
#include "esphome/core/esphal.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome
{
    namespace hx711
    {

        enum HX711Gain
        {
            HX711_GAIN_128 = 1,
            HX711_GAIN_32 = 2,
            HX711_GAIN_64 = 3,
        };

        class HX711Sensor : public sensor::Sensor, public PollingComponent
        {
        public:
            void set_dout_pin(GPIOPin *dout_pin) { dout_pin_ = dout_pin; }
            void set_sck_pin(GPIOPin *sck_pin) { sck_pin_ = sck_pin; }
            void set_gain(HX711Gain gain) { gain_ = gain; }
            void set_median_num(Median_Num median ) { median_ = median; }
            void set_median_time(Median_Loop_Time median_time) { median_time_ = median_time; }

            void setup() override;
            void dump_config() override;
            float get_setup_priority() const override;
            void update() override;

        protected:
            bool read_sensor_(uint32_t *result);

            GPIOPin *dout_pin_;
            GPIOPin *sck_pin_;
            HX711Gain gain_{HX711_GAIN_128};
            Median_Num *median_;
            Median_time *median_time_;
        };

    } // namespace hx711
} // namespace esphome
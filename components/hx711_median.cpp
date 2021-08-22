#include "hx711.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "RunningMedian.h"

namespace esphome
{
    namespace hx711
    {

        static const char *const TAG = "hx711";

        void HX711Sensor::setup()
        {
            ESP_LOGCONFIG(TAG, "Setting up HX711 '%s'...", this->name_.c_str());
            this->sck_pin_->setup();
            this->dout_pin_->setup();
            this->sck_pin_->digital_write(false);
            this->median_->setup();
            this->median_time_->setup();

            // Read sensor once without publishing to set the gain
            this->read_sensor_(nullptr);
        }

        void HX711Sensor::dump_config()
        {
            LOG_SENSOR("", "HX711", this);
            LOG_PIN("  DOUT Pin: ", this->dout_pin_);
            LOG_PIN("  SCK Pin: ", this->sck_pin_);
            LOG_PIN("  Median Count: ", this->median_);
            LOG_PIN("  Median Loop Time: ", this->median_time_);
            LOG_UPDATE_INTERVAL(this);
        }
        float HX711Sensor::get_setup_priority() const { return setup_priority::DATA; }

        void HX711Sensor::update() // this value is what is returned to esphome
        {
            RunningMedian median_result = RunningMedian(this->median_);
            uint8_t i = 0;
            uint32_t result;
            uint32_t median_result;
            for (i = 0; i <= this->median_; i++)
            {
                if (this->read_sensor_(&result)) //value is converted inside this function and is returned as a number here 
                {
                    int32_t value = static_cast<int32_t>(result);
                    ESP_LOGD(TAG, "'%s': Got value %d", this->name_.c_str(), value);
                    this->publish_state(value);
                }

                median_result.add(result);
                delay(this->median_time_);
            }
        }

        bool HX711Sensor::read_sensor_(uint32_t *result)
        {
            if (this->dout_pin_->digital_read())
            {
                ESP_LOGW(TAG, "HX711 is not ready for new measurements yet!");
                this->status_set_warning();
                return false;
            }

            this->status_clear_warning();
            uint32_t data = 0;

            {
                InterruptLock lock;
                for (uint8_t i = 0; i < 24; i++)
                {
                    this->sck_pin_->digital_write(true); //calls sck pin to say we want a reading
                    delayMicroseconds(1);
                    data |= uint32_t(this->dout_pin_->digital_read()) << (23 - i);
                    this->sck_pin_->digital_write(false);
                    delayMicroseconds(1);
                }

                // Cycle clock pin for gain setting
                for (uint8_t i = 0; i < this->gain_; i++)
                {
                    this->sck_pin_->digital_write(true);
                    delayMicroseconds(1);
                    this->sck_pin_->digital_write(false);
                    delayMicroseconds(1);
                }
            }

            if (data & 0x800000ULL)
            {
                data |= 0xFF000000ULL;
            }

            if (result != nullptr)
                *result = data;
            return true;
        }

    } // namespace hx711
} // namespace esphome
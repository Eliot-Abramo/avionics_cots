/*
* ADS1234_Mass_Thread.hpp
* @author Eliot Abramo
*/


#include "monitor.hpp"
#include "packet_definition.hpp"
#include "Cosco.hpp"
#include "HX711.h"

 
class HX711Thread {
public:
    HX711Thread();
    ~HX711Thread();

    void init();
    void loop();

    /**
     * @brief Start calibration offset
     * @param num_samples Number of samples to take for calibration
     * @return null
     */
    void start_calib_offset(uint32_t num_samples);

    /**
     * @brief Start calibration scale
     * @param num_samples Number of samples to take for calibration
     * @param calib_weight Weight to use for calibration
     * @return null
     */
    void start_calib_scale(uint32_t num_samples, float calib_weight);

    bool configured = false;

private:


    HX711* mass_sensor;
    
    float alpha = 0.8;
    uint16_t  num_averages = 10;
    long unsigned int start = 0;
    bool calibrating = false;

    long unsigned int config_time = 0;
    long unsigned int config_req_interval = 5000;

    void request_config();

    // Calibration parameters

    bool calibrating_offset = false;;
    bool calibrating_scale = false;

    uint32_t cnt_mass_offset = 0;
    uint32_t cnt_mass_scale = 0;

    uint32_t calib_samples_offset = 50;
    uint32_t calib_samples_scale = 50;

    float calib_weight = 0;

    float mass_avg_offset;
    double mass_sum_offset;

    float mass_avg_scale;
    double mass_sum_scale;

    void send_calib_offset();
    void send_calib_scale();

    SerialMonitor mass_monitor;
    Cosco mass_handler;
};

extern HX711Thread* MassSensorInstance;

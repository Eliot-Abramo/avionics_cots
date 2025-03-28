/*
* ADS1234_Mass_Thread.hpp
* @author Eliot Abramo
*/


#include "monitor.hpp"
#include "packet_definition.hpp"
#include "Cosco.hpp"
#include "HX711.h"
#include <iostream>

/*
    This is just a testing snippet to see if the calibration works
    Comment out other prints to ease debugging
    It is not used in the final code

    1. Declare a bool to block an scale calibration just after doing one
    static bool already_calibrated = false;

    2. Add this in the class "private:" part

    long unsigned int start = 0;

    3: Put this on the loop
    
 	if(esp_timer_get_time()-start > 10000000){
    	start = esp_timer_get_time();
		if (!already_calibrated) {
			start_calib_offset(5);
		}
		if (!calibrating_offset) {
			start_calib_scale(50, 75.0);
		}
    	mass_monitor.log("Calibrating mass sensor...");
		mass_monitor.log(message);
    }
*/
 
class HX711Thread {
public:
    HX711Thread();
    ~HX711Thread();

    void init();
    void loop();

    /**
     * @brief Start calibration offset. Average out the sensor reading for a number of samples and put that as an OFFSEt
     * @param num_samples Number of samples to take for calibration
     * @return null
     */
    void start_calib_offset(uint32_t num_samples);

    /**
     * @brief Start calibration scale. The calib_weight is the weight to use for calibration, our know reference weight.
     * This sets the SCALE = sensor reading / known value
     * This "known value" needs to be in the units we want to output which is grams.
     * @param num_samples Number of samples to average to use for calibration
     * @param calib_weight Weight to use for calibration
     * @return null
     */
    void start_calib_scale(uint32_t num_samples, float calib_weight);

    bool configured = false;

private:

    HX711* mass_sensor;


    //Time variables for requesting configuration to CS
    uint64_t config_time = 0;
    uint64_t config_req_interval = 5000;

    /**
     * @brief Request configuration from CS. This is done by sending a packet to CS with the ID of the mass sensor
     * and the request for offset, scale and status. The CS will then respond with the configuration.
     * @return null
     */

    void request_config();

    // Calibration parameters

    //WARNING. Always set offset before scale.

    
    //Offset parameters

    bool calibrating_offset = false; //Bool controlling offset calibration

    uint32_t cnt_mass_offset = 0; // Counter for number of samples taken

    uint32_t calib_samples_offset = 5; // Number of samples to take for offset calibration. WARNING. Initialized to 5 bigger numbers can cause an overflow.

    double mass_sum_offset; //Double to accumulate samples for offset calibration
    float mass_avg_offset; //Float to store the average of the summed samples for offset calibration


    //Scale parameters

    bool calibrating_scale = false; //Bool controlling scale calibration

    uint32_t cnt_mass_scale = 0; // Counter for number of samples taken

    uint32_t calib_samples_scale = 50; // Number of samples to take for scale calibration if not overwritten

    float calib_weight = 0; // Known weight to use for scale calibration
 
    double mass_sum_scale; //Double to accumulate samples for scale calibration
    float mass_avg_scale; //Float to store the average of the summed samples for scale calibration


    /**
     * @brief Called some time after start_calib_offset(). This is the functuon actually computing the division of the average (sum/number of samples)
     * and setting the offset to that value. It does this after the loop sums over enough samples as defined in the start_calib_offset function. 
     * It also sends a packet to CS with the new offset value.
     * @return null
     */
    void send_calib_offset();
    /**
     * @brief Called some time after start_calib_scale(). This is the functuon actually computing the division of the average (sum/(number of samples*known weight))
     * and setting the scale to that value. It does this after the loop sums over enough samples as defined in the start_calib_scale function. 
     * It also sends a packet to CS with the new scale value.
     * @return null
     */
    void send_calib_scale();

    SerialMonitor mass_monitor;
    Cosco mass_handler;
};

extern HX711Thread* MassSensorInstance;

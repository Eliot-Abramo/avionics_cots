/*
* ADS1234_Mass_Thread.cpp
* @author Eliot Abramo
*/

#include <iostream>
#include "ADS1234_Mass_Thread.hpp"

#define PIN_SCLK 17
#define PIN_PDWN 19
#define PIN_SPEED 18
#define PIN_GAIN0 2
#define PIN_GAIN1 5
#define PIN_A0 2
#define PIN_A1 2
#define PIN_DOUT 2 

ADS1234Thread* MassSensorInterface = nullptr;
static char cbuf[256];

static MassData mass_data;
static MassConfigRequestPacket mass_config_packet;
static MassConfigResponsePacket mass_calib_offset_response_packet = {};
static MassConfigResponsePacket mass_calib_scale_response_packet = {};

ADS1234Thread::ADS1234Thread() : mass_sensor(nullptr)
{
    mass_sensor = new ADS1234(PIN_SCLK, PIN_PDWN, PIN_SPEED, PIN_GAIN0, 
                                PIN_GAIN1, PIN_A0, PIN_A1, PIN_DOUT);
}

ADS1234Thread::~ADS1234Thread()
{
    delete mass_sensor;
    mass_sensor = nullptr;
}

void ADS1234Thread::init()
{
    MassSensorInterface = this;
    
    mass_monitor.log("Mass thread created");

    mass_sensor->begin();
    // Channel 1
    mass_sensor->set_offset(AIN1, 263616.4375);
	mass_sensor->set_scale(AIN1, 451.8433);
    // Channel 2
    mass_sensor->set_offset(AIN2, 263616.4375);
	mass_sensor->set_scale(AIN2, 451.8433);
    // //Channel 3
    mass_sensor->set_offset(AIN3, 263616.4375);
	mass_sensor->set_scale(AIN3, 451.8433);
    // //Channel 4
    mass_sensor->set_offset(AIN4, 263616.4375);
	mass_sensor->set_scale(AIN4, 451.8433);

    request_config();
}

// Sends mass configuration packet
void ADS1234Thread::request_config()
{
    mass_monitor.log("Requesting configuration...");
    config_time = esp_timer_get_time();
	mass_config_packet.req_offset = true;
	mass_config_packet.req_scale = true;
	mass_config_packet.req_alpha = true;
	mass_config_packet.req_channels_status = true;

	mass_handler.sendMassConfigRequestPacket(&mass_config_packet);
	// MAKE_IDENTIFIABLE(mass_config_packet);
	// MAKE_RELIABLE_MCU(mass_config_packet);
	// Telemetry::set_id(JETSON_NODE_ID);
	// FDCAN1_network->send(&mass_config_packet);
	// FDCAN2_network->send(&mass_config_packet);
	// portYIELD();
}

void ADS1234Thread::start_calib_offset(uint32_t num_samples, uint8_t channel)
{
    if(channel <= 4) {
        if(!calibrating_offset) {
            if(!calibrating_scale) {
                calib_channel = channel;
                cnt_mass_offset = 0;
                
                for(uint8_t i=0; i<4; ++i) {
                    mass_avg_offset[i] = 0;
                }

                calib_samples_offset = num_samples;
                calibrating_offset = true;
                mass_monitor.log("Starting offset calibration");
            } else {
                mass_monitor.log("Cannot calibrate both offset and scale at the same time");
            }
        } else {
            mass_monitor.log("Another offset calibration is already active. Aborting calculation...");
        }
    } else {
        String message = "Invalid channel number for offset calibration: %d", channel;
		mass_monitor.log(message);
	}
}

void ADS1234Thread::start_calib_scale(uint32_t num_samples, uint8_t channel, float calib_weight) 
{
	if (channel <= 4) {
		if (!calibrating_scale) {
			if (!calibrating_offset) {
				calib_channel = channel;
				cnt_mass_scale = 0;
				this->calib_weight = calib_weight;
				
                for (uint8_t i = 0; i < 4; ++i){
					mass_avg_scale[i] = 0;
                }

                calib_samples_scale = num_samples;
				calibrating_scale = true;
				mass_monitor.log("Starting scale calibration...");
			} else {
				mass_monitor.log("Cannot calibrate both offset and scale at the same time");
			}
		} else {
			mass_monitor.log("Another scale calibration is already active. Aborting calibration...");
		}
	} else {
        String message = "Invalid channel number for scale calibration: %d", channel;
		mass_monitor.log(message);
	}
}

void ADS1234Thread::loop()
{
    // Request configuration
    if((esp_timer_get_time() - config_time > config_req_interval) && !configured) {
		request_config();
	}

	// Calibrate every 90 seconds
	ERROR_t err_ch1 = NoERROR;
	ERROR_t err_ch2 = NoERROR;
	ERROR_t err_ch3 = NoERROR;
	ERROR_t err_ch4 = NoERROR;

	if(esp_timer_get_time()-start > 90000){
   	calibrating = true;
   	start = esp_timer_get_time();
   	mass_monitor.log("Calibrating mass sensor...");
   }

   #ifdef USE_LOW_PASS_FILTER
	if (enabled_channels[0])
    	err_ch1 = mass_sensor->get_units(AIN1, mass_data.mass[0], alpha, calibrating);

	if (enabled_channels[1])
    	err_ch2 = mass_sensor->get_units(AIN2, mass_data.mass[1], alpha, calibrating);

	if (enabled_channels[2])
    	err_ch3 = mass_sensor->get_units(AIN3, mass_data.mass[2], alpha, calibrating);

	if (enabled_channels[3])
    	err_ch4 = mass_sensor->get_units(AIN4, mass_data.mass[3], alpha, calibrating);
#elif
	if (enabled_channels[0])
    	err_ch1 = mass_sensor->get_units(AIN1, mass_data.mass[0], num_averages, calibrating);

	if (enabled_channels[1])
    	err_ch2 = mass_sensor->get_units(AIN2, mass_data.mass[1], num_averages, calibrating);

	if (enabled_channels[2])
    	err_ch3 = mass_sensor->get_units(AIN3, mass_data.mass[2], num_averages, calibrating);

	if (enabled_channels[3])
    	err_ch4 = mass_sensor->get_units(AIN4, mass_data.mass[3], num_averages, calibrating);
#endif

// #ifdef PLOT_CH1
//     global_mass_ch1 = mass_data.mass[0];
// #endif
// #ifdef PLOT_CH2
//     global_mass_ch2 = mass_data.mass[1];
// #endif
// #ifdef PLOT_CH3
//     global_mass_ch3 = mass_data.mass[2];
// #endif
// #ifdef PLOT_CH4
//     global_mass_ch2 = mass_data.mass[3];
// #endif

    if (calibrating) {
    	calibrating = false;
    }

	if((err_ch1 == NoERROR) && (err_ch2 == NoERROR) && (err_ch3 == NoERROR) && (err_ch4 == NoERROR)) {
		String message = mass_data.toString(cbuf);
        mass_monitor.log(message);
	
		// Calibration
		if(calibrating_offset) {
			cnt_mass_offset += 1;
			mass_sum_offset[0] += mass_sensor->get_last_filtered_raw(AIN1);
			mass_sum_offset[1] += mass_sensor->get_last_filtered_raw(AIN2);
			mass_sum_offset[2] += mass_sensor->get_last_filtered_raw(AIN3);
			mass_sum_offset[3] += mass_sensor->get_last_filtered_raw(AIN4);
		}

		if(calibrating_scale) {
			cnt_mass_scale += 1;
			mass_sum_scale[0] += mass_sensor->get_last_filtered_tared(AIN1);
			mass_sum_scale[1] += mass_sensor->get_last_filtered_tared(AIN2);
			mass_sum_scale[2] += mass_sensor->get_last_filtered_tared(AIN3);
			mass_sum_scale[3] += mass_sensor->get_last_filtered_tared(AIN4);
		}

		if(calibrating_offset && (cnt_mass_offset > calib_samples_offset)) {
			send_calib_offset();
		}

		if(calibrating_scale && (cnt_mass_scale > calib_samples_scale)) {
			send_calib_scale();
		}

		mass_data.toArray((uint8_t*) &mass_data);
		// MAKE_IDENTIFIABLE(mass_packet);
		// MAKE_RELIABLE_MCU(mass_packet);
		// Telemetry::set_id(JETSON_NODE_ID);
		// FDCAN1_network->send(&mass_packet);
		// FDCAN2_network->send(&mass_packet);
		// portYIELD();
		mass_handler.sendMassDataPacket(&mass_data);

	} else {
		mass_monitor.log("Thread aborted");
		MassSensorInterface = nullptr;
		delete mass_sensor;
		mass_sensor = nullptr;
		// if (hspi == &hspi1)
		// 	MX_SPI1_Init();
		// else if (hspi == &hspi2)
		// 	MX_SPI2_Init();
		// else if (hspi == &hspi3)
		// 	MX_SPI3_Init();
		// terminate();
		// parent->resetProber();
	}
}


void ADS1234Thread::send_calib_offset() {
	// Compute average value

	for (uint8_t i = 0; i < 4; ++i) {
		if (this->enabled_channels[i] && (cnt_mass_offset != 0))
			mass_avg_offset[i] = mass_sum_offset[i]/cnt_mass_offset;
	}

	calibrating_offset = false;
	cnt_mass_offset = 0;
	for (uint8_t i = 0; i < 4; ++i)
		mass_sum_offset[i] = 0;

	mass_calib_offset_response_packet.set_offset = true;

	if (this->enabled_channels[0] && ((calib_channel == 1) || (calib_channel == 0)))
		mass_sensor->set_offset(AIN1, mass_avg_offset[0]);

	if (this->enabled_channels[1] || ((calib_channel == 2) || (calib_channel == 0)))
			mass_sensor->set_offset(AIN2, mass_avg_offset[1]);

	if (this->enabled_channels[2] || ((calib_channel == 3) || (calib_channel == 0)))
			mass_sensor->set_offset(AIN3, mass_avg_offset[2]);

	if (this->enabled_channels[3] || ((calib_channel == 4) || (calib_channel == 0)))
			mass_sensor->set_offset(AIN4, mass_avg_offset[3]);


	mass_calib_offset_response_packet.offset[0] = mass_sensor->get_offset(AIN1);
	mass_calib_offset_response_packet.offset[1] = mass_sensor->get_offset(AIN2);
	mass_calib_offset_response_packet.offset[2] = mass_sensor->get_offset(AIN3);
	mass_calib_offset_response_packet.offset[3] = mass_sensor->get_offset(AIN4);

	snprintf(cbuf, sizeof(cbuf), "Computed mass sensor offset: [%.3f %.3f %.3f %.3f]",
			mass_sensor->get_offset(AIN1), mass_sensor->get_offset(AIN2),
			mass_sensor->get_offset(AIN3), mass_sensor->get_offset(AIN4));
	String message = String(cbuf);

	mass_monitor.log(message);

	for (uint8_t i = 0; i < 4; ++i) {
		mass_calib_offset_response_packet.enabled_channels[i] = enabled_channels[i];
	}

	mass_handler.sendMassConfigResponsePacket(&mass_calib_offset_response_packet);
	// MAKE_IDENTIFIABLE(mass_calib_offset_response_packet);
	// MAKE_RELIABLE_MCU(mass_calib_offset_response_packet);
	// Telemetry::set_id(JETSON_NODE_ID);
	// if (sender_id == 1)
	// 	FDCAN1_network->send(&mass_calib_offset_response_packet);
	// else if (sender_id == 2)
	// 	FDCAN2_network->send(&mass_calib_offset_response_packet);
	// portYIELD();

}

void ADS1234Thread::send_calib_scale() {
	// Compute average value

	for (uint8_t i = 0; i < 4; ++i) {
		if (this->enabled_channels[i] && (cnt_mass_scale != 0) && (calib_weight != 0)) {
			mass_avg_scale[i] = mass_sum_scale[i]/(cnt_mass_scale*calib_weight);
		}
	}

	calibrating_scale = false;
	cnt_mass_scale = 0;
	for (uint8_t i = 0; i < 4; ++i)
		mass_sum_scale[i] = 0;

	mass_calib_scale_response_packet.set_scale = true;

	if (this->enabled_channels[0] && ((calib_channel == 1) || (calib_channel == 0)))
		mass_sensor->set_scale(AIN1, mass_avg_scale[0]);

	if (this->enabled_channels[1] || ((calib_channel == 2) || (calib_channel)))
			mass_sensor->set_scale(AIN2, mass_avg_scale[1]);

	if (this->enabled_channels[2] || ((calib_channel == 3) || (calib_channel == 0)))
			mass_sensor->set_scale(AIN3, mass_avg_scale[2]);

	if (this->enabled_channels[3] || ((calib_channel == 4) || (calib_channel == 0)))
			mass_sensor->set_scale(AIN4, mass_avg_scale[3]);


	mass_calib_scale_response_packet.scale[0] = mass_sensor->get_scale(AIN1);
	mass_calib_scale_response_packet.scale[1] = mass_sensor->get_scale(AIN2);
	mass_calib_scale_response_packet.scale[2] = mass_sensor->get_scale(AIN3);
	mass_calib_scale_response_packet.scale[3] = mass_sensor->get_scale(AIN4);

	snprintf(cbuf, sizeof(cbuf), "Computed mass sensor scale: [%.3f %.3f %.3f %.3f]",
		mass_sensor->get_scale(AIN1), mass_sensor->get_scale(AIN2),
		mass_sensor->get_scale(AIN3), mass_sensor->get_scale(AIN4));
	
	String message = String(cbuf);
	mass_monitor.log(message);

	for (uint8_t i = 0; i < 4; ++i) {
		mass_calib_offset_response_packet.enabled_channels[i] = enabled_channels[i];
	}

	mass_handler.sendMassConfigResponsePacket(&mass_calib_scale_response_packet);
	// MAKE_IDENTIFIABLE(mass_calib_scale_response_packet);
	// MAKE_RELIABLE_MCU(mass_calib_scale_response_packet);
	// Telemetry::set_id(JETSON_NODE_ID);
	// if (sender_id == 1)
	// 	FDCAN1_network->send(&mass_calib_scale_response_packet);
	// else if (sender_id == 2)
	// 	FDCAN2_network->send(&mass_calib_scale_response_packet);
	// portYIELD();

}
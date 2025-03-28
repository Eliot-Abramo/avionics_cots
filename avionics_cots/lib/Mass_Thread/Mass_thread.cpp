/*
* ADS1234_Mass_Thread.cpp
* @author Eliot Abramo
*/

#include <iostream>
#include "Mass_thread.hpp"

/**
 * Independently tested functionalities:
 * - init/loop
 * - start_calib_offset
 * - start_calib_scale
 * - send_calib_offset (used by start_calib_offset)
 * - send_calib_scale (used by start_calib_scale)
 */


HX711Thread* MassSensorInterface = nullptr;
static char cbuf[256];

#define PIN_SCLK 4
#define PIN_DOUT 16

static MassData mass_data;
static MassConfigRequestPacket mass_config_packet;
static MassConfigResponsePacket mass_calib_offset_response_packet = {};
static MassConfigResponsePacket mass_calib_scale_response_packet = {};

HX711Thread::HX711Thread() : mass_sensor(nullptr)
{
    mass_sensor = new HX711(PIN_SCLK, PIN_DOUT);
}

HX711Thread::~HX711Thread()
{
    delete mass_sensor;
    mass_sensor = nullptr;
}

void HX711Thread::init()
{
    MassSensorInterface = this;
    
    mass_monitor.log("Mass thread created");

    mass_sensor->begin();

    //request_config();
}

// Sends mass configuration packet
void HX711Thread::request_config()
{
  	mass_monitor.log("Requesting configuration...");
    config_time = esp_timer_get_time();
	mass_config_packet.req_offset = true;
	mass_config_packet.req_scale = true;

	mass_handler.sendMassConfigRequestPacket(&mass_config_packet);	
}

void HX711Thread::start_calib_offset(uint32_t num_samples)
{

	if (!calibrating_offset) {
		if (!calibrating_scale) {

			//Reset values
			cnt_mass_offset = 0;
			mass_avg_offset = 0;

			//Set the number of samples to take
      		calib_samples_offset = num_samples;

			//Enable offset calibration
			calibrating_offset = true;

			//Reset the current offset
			mass_sensor->set_offset(0);

			mass_monitor.log("Starting offset calibration...");
	  	} else {
		  mass_monitor.log("Cannot calibrate both offset and scale at the same time");
		}
  } else {
      mass_monitor.log("Another offset calibration is already active. Aborting calibration...");
  }
}

void HX711Thread::start_calib_scale(uint32_t num_samples, float calib_weight) 
{

	if (!calibrating_scale) {
		if (!calibrating_offset) {

			//Reset values
			cnt_mass_scale = 0;
			mass_avg_scale = 0;

			//Update the calibration known weight
			this->calib_weight = calib_weight;

			//Set the number of samples to take
      		calib_samples_scale = num_samples;

			//Enable scale calibration
			calibrating_scale = true;

			//Reset the current scale
			mass_sensor->set_scale(1.0);

			mass_monitor.log("Starting scale calibration...");
	  	} else {
		  mass_monitor.log("Cannot calibrate both offset and scale at the same time");
		}
  } else {
      mass_monitor.log("Another scale calibration is already active. Aborting calibration...");
  }
}

void HX711Thread::loop()
{	/*
    // Request configuration
  	if((esp_timer_get_time() - config_time > config_req_interval) && !configured) {
		//request_config();
	}
	*/

	// Get mass data
	mass_data.mass = mass_sensor->get_units(10);

	// Format data to print or send in a packet
	mass_data.toArray((uint8_t*) &mass_data);		
	String message = mass_data.toString(cbuf);

	// Calibration$
	// Accumulate values for offset and scale calibration
	if(calibrating_offset) {
		cnt_mass_offset += 1;
		mass_sum_offset += mass_sensor->get_units(10);
	}

	if(calibrating_scale) {
		cnt_mass_scale += 1;
		mass_sum_scale += mass_sensor->get_units(10);
	}

	//Sending cummulative values
	if(calibrating_offset && (cnt_mass_offset > calib_samples_offset)) {
		send_calib_offset();
	}

	if(calibrating_scale && (cnt_mass_scale > calib_samples_scale)) {
		send_calib_scale();
	}

	
	mass_monitor.log(message);
	//mass_handler.sendMassDataPacket(&mass_data);

}


void HX711Thread::send_calib_offset() {
	// Compute average value
	if ((cnt_mass_offset != 0)) {
		mass_avg_offset = mass_sum_offset/cnt_mass_offset;
	}

	//Deactivate offset calibration
	calibrating_offset = false;

	//Reset values to begin a new offset averaging calculation when needed
	cnt_mass_offset = 0;
	mass_sum_offset = 0;

	//Set the value we computed
	mass_sensor->set_offset(static_cast<long>(mass_avg_offset));

	//Put on a packet to notify CS that we set the offset
	mass_calib_offset_response_packet.set_offset = true;

	//Actually put the offset in the packet
	mass_calib_offset_response_packet.offset = mass_sensor->get_offset();

	//Print the offset value
	snprintf(cbuf, sizeof(cbuf), "Computed mass sensor offset: [%ld]", mass_sensor->get_offset());
	String message = String(cbuf);
	mass_monitor.log(message);

	//Send the packet to CS
  	//mass_handler.sendMassConfigResponsePacket(&mass_calib_offset_response_packet);
}

void HX711Thread::send_calib_scale() {

	// Compute average value

	if ((cnt_mass_scale != 0) && (calib_weight != 0)) {
		mass_avg_scale = mass_sum_scale/(cnt_mass_scale*calib_weight);
	}

	//Deactivate scale calibration
	calibrating_scale = false;

	//Reset values to begin a new scale averaging calculation when needed
	cnt_mass_scale = 0;
	mass_sum_scale = 0;

	//Set the value we computed
	mass_sensor->set_scale(mass_avg_scale);

	//Put on a packet to notify CS that we set the scale
	mass_calib_scale_response_packet.set_scale = true;

	//Actually put the scale in the packet
	mass_calib_scale_response_packet.scale = mass_sensor->get_scale();

	//Print the scale value
	snprintf(cbuf, sizeof(cbuf), "Computed mass sensor scale: [%.3f]", mass_sensor->get_scale());
	
	String message = String(cbuf);
	mass_monitor.log(message);

	//Send the packet to CS
	//mass_handler.sendMassConfigResponsePacket(&mass_calib_scale_response_packet);
}
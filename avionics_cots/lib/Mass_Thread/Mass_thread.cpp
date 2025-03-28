/*
* ADS1234_Mass_Thread.cpp
* @author Eliot Abramo
*/

#include <iostream>
#include "Mass_thread.hpp"


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
	mass_monitor.log("TEST1 FAILED");
  	mass_monitor.log("Requesting configuration...");
    config_time = esp_timer_get_time();
	mass_config_packet.req_offset = true;
	mass_config_packet.req_scale = true;

	mass_handler.sendMassConfigRequestPacket(&mass_config_packet);	
}

void HX711Thread::start_calib_offset(uint32_t num_samples)
{
  mass_monitor.log("TEST1 FAILED");
  if(!calibrating_offset) {
  if(!calibrating_scale) {
    cnt_mass_offset = 0;

    mass_avg_offset = 0;

    calib_samples_offset = num_samples;
    calibrating_offset = true;
    mass_monitor.log("Starting offset calibration");
    } else {
      mass_monitor.log("Cannot calibrate both offset and scale at the same time");
    }
  } else {
    mass_monitor.log("Another offset calibration is already active. Aborting calculation...");
  }
}

void HX711Thread::start_calib_scale(uint32_t num_samples, float calib_weight) 
{
	mass_monitor.log("TEST1 FAILED");	
	if (!calibrating_scale) {
		if (!calibrating_offset) {
			cnt_mass_scale = 0;
			this->calib_weight = calib_weight;
			mass_avg_scale = 0;
      calib_samples_scale = num_samples;
			calibrating_scale = true;
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

 	if(esp_timer_get_time()-start > 90000){
    	calibrating = true;
    	start = esp_timer_get_time();
    	mass_monitor.log("Calibrating mass sensor...");
    }

    if (calibrating) {
    	calibrating = false;
    }
	*/
	mass_data.mass = mass_sensor->get_units(10);

	String message = mass_data.toString(cbuf);
    mass_monitor.log(message);
	
	// Calibration
	if(calibrating_offset) {
		cnt_mass_offset += 1;
		mass_sum_offset += mass_sensor->get_units(10);
	}

	if(calibrating_scale) {
		cnt_mass_scale += 1;
		mass_sum_scale += mass_sensor->get_units(10);
	}

	if(calibrating_offset && (cnt_mass_offset > calib_samples_offset)) {
		send_calib_offset();
	}

	if(calibrating_scale && (cnt_mass_scale > calib_samples_scale)) {
		send_calib_scale();
	}

	mass_data.toArray((uint8_t*) &mass_data);		
	mass_handler.sendMassDataPacket(&mass_data);
	String data = mass_data.toString(cbuf);
	Serial.println(data);
}


void HX711Thread::send_calib_offset() {
	mass_monitor.log("TEST1 FAILED");
	// Compute average value
	mass_avg_offset = mass_sum_offset/cnt_mass_offset;


	calibrating_offset = false;
	cnt_mass_offset = 0;
	mass_sum_offset = 0;

	mass_calib_offset_response_packet.set_offset = true;

  	mass_sensor->set_offset(mass_avg_offset);

	mass_calib_offset_response_packet.offset = mass_sensor->get_offset();

	snprintf(cbuf, sizeof(cbuf), "Computed mass sensor offset: [%.3f]", mass_sensor->get_offset());
	String message = String(cbuf);

	mass_monitor.log(message);
  	mass_handler.sendMassConfigResponsePacket(&mass_calib_offset_response_packet);
}

void HX711Thread::send_calib_scale() {
	mass_monitor.log("TEST1 FAILED");

	// Compute average value

	if ((cnt_mass_scale != 0) && (calib_weight != 0)) {
		mass_avg_scale = mass_sum_scale/(cnt_mass_scale*calib_weight);
	}


	calibrating_scale = false;
	cnt_mass_scale = 0;
	mass_sum_scale = 0;

	mass_calib_scale_response_packet.set_scale = true;

	mass_sensor->set_scale(mass_avg_scale);

	mass_calib_scale_response_packet.scale = mass_sensor->get_scale();

	snprintf(cbuf, sizeof(cbuf), "Computed mass sensor scale: [%.3f]", mass_sensor->get_scale());
	
	String message = String(cbuf);
	mass_monitor.log(message);

	mass_handler.sendMassConfigResponsePacket(&mass_calib_scale_response_packet);
}
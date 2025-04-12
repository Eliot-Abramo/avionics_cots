/**
 * @file Servo.cpp
 * @author Eliot Abramo
*/
#include "Servo.hpp"

Servo_Driver::Servo_Driver()
{
    servoRequest = new ServoRequest();
    servoResponse = new ServoResponse();

    min_angle = -200;
	max_angle = 200;
	min_pulse = 500;
	max_pulse = 2500;

    zero_pulse = 1500;
    angle = 0;

}
Servo_Driver::~Servo_Driver()
{
    delete servoRequest;
    servoRequest = nullptr;
    delete servoResponse;
    servoResponse = nullptr;
}

void Servo_Driver::init(int8_t servoPin, uint8_t channel)
{
    _channel = channel;
    ledcSetup(channel, 50, 16);
    ledcAttachPin(servoPin, channel);
    zero_in();
}

void Servo_Driver::zero_in()
{
    float duty_cycle = (zero_pulse * 65535) / 20000;
    ledcWrite(_channel, duty_cycle);
}

float Servo_Driver::angle_to_duty()
{
    float pwm = map(angle, min_angle, max_angle, min_pulse, max_pulse);
    float duty_cycle = (pwm * 65535) / 20000;
    return duty_cycle;
}

void Servo_Driver::set_servo()
{
    ledcWrite(_channel, angle_to_duty());
}

std::vector<std::string> split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;
  
    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }
  
    res.push_back (s.substr (pos_start));
    return res;
  }


void Servo_Driver::handle_servo() {
    // std::string to_parse = buffer;
    // std::vector<std::string> parsed = split(to_parse, ",");

    if (servoRequest != nullptr) {
        int32_t new_angle = angle + servoRequest->increment;
        if (servoRequest->zero_in) { //check if zero_in was requested
            angle = 0;
            zero_in();
            servoResponse->id = servoRequest->id;
            servoResponse->angle = 0; // cancel rotation and zero in
            servoResponse->success = true;
        } else if (!(new_angle <= max_angle and new_angle >= min_angle)) { //check for angle within bounds
            angle = new_angle <= min_angle ? min_angle : max_angle;
            set_servo();
            servoResponse->id = servoRequest->id;
            servoResponse->angle = angle;
            servoResponse->success = false;
        } else { //set servo to angle if OK
            angle = new_angle;
            set_servo();            
            servoResponse->id = servoRequest->id;
            servoResponse->angle = angle;
            servoResponse->success = true;
        }
    }
}

void Servo_Driver::set_request(ServoRequest req) {
    *servoRequest = req;
}

ServoResponse* Servo_Driver::get_response() {
    return servoResponse;
}


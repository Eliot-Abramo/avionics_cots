/**
 * @file packet_definition.hpp
 * @author Eliot Abramo
*/
#ifndef PACKET_DEFINITION_H
#define PACKET_DEFINITION_H

#include <iostream>
#include "packet_id.hpp"

struct MassConfigPacket
{
    uint8_t id=MassConfig_ID;
    float offset[4];
    float scale[4];
    float alpha;
    bool enabled_channels[4];
    bool remote_command;
    bool set_offset;
    bool set_scale;
    bool set_alpha;
    bool set_channels_status;
};

struct MassConfigRequestPacket
{
    uint8_t id=MassConfigRequest_ID;
    bool req_offset;
    bool req_scale;
    bool req_alpha;
    bool req_channels_status;
};

struct MassConfigResponsePacket
{
    uint8_t id=MassConfigResponse_ID;
    float offset[4];
    float scale[4];
    float alpha;
    bool enabled_channels[4];
    bool set_offset;
    bool set_scale;
    bool set_alpha;
    bool set_channels_status;
    bool success;
};  

struct MassData {
    float mass[4];
    char* toString(char* buffer) {
		sprintf(buffer, "CH1: %+.3f [g] \t CH2: %+.3f [g] \t CH3: %+.3f [g] \t CH4: %+.3f [g]", mass[0], mass[1], mass[2], mass[3]);
		return buffer;
	}

    uint8_t* toArray(uint8_t* buffer){
    	for(int i = 0; i < 4; ++i)
    		*(float*)(buffer + i * 4) = mass[i];
        return buffer;
    }
};

#endif /* PACKET_DEFINITION_H */

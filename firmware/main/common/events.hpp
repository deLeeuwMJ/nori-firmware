#pragma once

#include <cstdint>

#define ID_SUCCES           0x41
#define ID_VEHICLE_SPEED    0x0D

namespace Events
{
    enum CarEvent {
        UNKNOWN,
        VEHICLE_SPEED
    };

    struct CarEventResponse
    {
        uint8_t status;
        uint8_t command;
        uint8_t value; // Hex Value
    };

    struct CarEventData
    {
        CarEvent event;
        int8_t value; // Decimal Value
    };

    CarEventResponse parseRawData(uint8_t* data);
    CarEventData retrieveData(CarEventResponse response);
    bool isSuccesfull(CarEventResponse response);
}
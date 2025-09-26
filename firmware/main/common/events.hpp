#pragma once

#include <cstdint>
#include <string>

#define ID_SUCCES           "41"
#define ID_VEHICLE_SPEED    "0D"

namespace Events
{
    enum CarEvent {
        UNKNOWN,
        VEHICLE_SPEED
    };

    struct CarEventResponse
    {
        std::string status;
        std::string command;
        std::string value;
    };

    struct CarEventData
    {
        CarEvent event;
        std::string value;
    };

    CarEventResponse parseRawData(std::string value);
    CarEventData retrieveData(CarEventResponse response);
    std::string carEventToString(CarEvent event);
}
#include "events.hpp"

namespace Events
{
    /*
        Still need to fix event parsing and its only accepting one byte values
    */
    CarEventResponse parseRawData(std::string value) 
    {
        CarEventResponse response = {
            .status = value.substr(0,2),
            .command = value.substr(4,2),
            .value = value.substr(6,2),
        };
        
        return response;
    };

    CarEventData retrieveData(CarEventResponse response) 
    {
        CarEvent event = CarEvent::UNKNOWN;

        if (response.command == ID_VEHICLE_SPEED)
            event = CarEvent::VEHICLE_SPEED;

        CarEventData data = {
            .event = event,
            .value = response.value,
        };
        
        return data;
    };

    std::string carEventToString(CarEvent event)
    {
        switch (event) {
            case VEHICLE_SPEED:
                return "VEHICLE_SPEED";
            case UNKNOWN:
            default:
                return "UNKNOWN";
        };
    };
}
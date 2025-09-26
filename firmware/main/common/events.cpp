#include "events.hpp"

namespace Events
{
    // Only accepting 1 byte response right now
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
}
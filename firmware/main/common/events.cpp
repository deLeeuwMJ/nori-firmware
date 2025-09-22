#include "events.hpp"

namespace Events
{
    // Only accepting 1 byte response right now
    CarEventResponse parseRawData(uint8_t* data) 
    {
        CarEventResponse response = {
            .status = data[0],
            .command = data[1],
            .value = data[2],
        };
        
        return response;
    };

    CarEventData retrieveData(CarEventResponse response) 
    {
        CarEvent event = CarEvent::UNKNOWN;

        switch (response.command)
        {
            case ID_VEHICLE_SPEED:
                event = CarEvent::VEHICLE_SPEED;
                break;
        }

        CarEventData data = {
            .event = event,
            .value = static_cast<int8_t>(response.value),
        };
        
        return data;
    };

    bool isSuccesfull(CarEventResponse response)
    {
        return response.status == ID_SUCCES;
    }
}
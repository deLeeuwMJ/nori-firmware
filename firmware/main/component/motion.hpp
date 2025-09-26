#pragma once

#include "functional"
#include "qmi8658.h"

#define TASK_MOTION              "TASK_MOTION"
#define LOG_TAG_MOTION_EVENT     "TASK_MOTION_EVENT"
struct GyroData
{
    float x;
    float y;
    float z;
};

struct AccelerometerData
{
    float x;
    float y;
    float z;
};

struct MotionEventData
{
    GyroData gyro;
    AccelerometerData accelerometer;
};

class Motion
{
    public:
        void setup(i2c_master_bus_handle_t i2c_bus_handle, std::function<void(MotionEventData data)> callback);
    private:
        static qmi8658_dev_t dev;

        static void processMotionEventTask(void *pvParameter);
        static std::function<void(MotionEventData data)> onResult;
};

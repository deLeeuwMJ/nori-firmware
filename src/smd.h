#define LED_FREQ 12000
#define LED_RESOLUTION 4
#define LED_BRIGHTNESS 200

#define RED_LED_PIN 23
#define RED_LED_CH 0

class SMDPeripheral {
    public:
        void init();
        void setRedValue(int brightness);
};

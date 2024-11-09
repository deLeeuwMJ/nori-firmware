#define BUZZER_PIN 4
#define NOTE 1047
#define NOTE_DURATION 500

class BuzzerPeripheral {
    public:
        void playTooHighSpeed();
    private:
        void beep();
};
#include <Arduino.h>
#include "buzzer.h"

void BuzzerPeripheral::playTooHighSpeed() {
    beep();
}

void BuzzerPeripheral::beep() {
    tone(BUZZER_PIN, NOTE, NOTE_DURATION);
}
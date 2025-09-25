#include "core/BlePeripheral.hpp"

extern "C" void app_main(void) {
    core::BluetoothPeripheral* peripheral = new core::BluetoothPeripheral();
    peripheral->setup();
}
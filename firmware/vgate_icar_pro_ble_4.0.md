# Vgate iCar Pro Bluetooth 4.0(BLE)
These are the results of an investigation done on the BLE tech stack with LightBlue App and chrome://bluetooth-internals

## Device
When connecting there are two devices discoverable:
- IOS-Vlink, 72:1C:62:B8:B0:AB/D2:e0:2F:8D:54:8F (device under investigation)
- Android-Vlink, 13:E0:2F:8D:54:8F (not tested)

It is still unclear if these are PUBLIC or RANDOM addresses.



When a connection is made, a BLUE led will turn on.

## Service
The service we are interested in is "e7810a71-73ae-499d-8c15-faa9aef0c3f2", at the time it had the ID "e7810a71-73ae-499d-8c15-faa9aef0c3f2-0x10c0d5c33e0"

## Characteristic
The characteristic we are interested in is "bef8d6c9-9c21-4c9e-b632-bd58c1009f9f", at the time it had the ID "bef8d6c9-9c21-4c9e-b632-bd58c1009f9f-0x10c10ccb9f0"
It supports Read, Write /w response, Write, Notify and Indicate.
In addition it only accepts UTF8-String messages.

In order to communicate, two things needs to be done:
- Write, to send ELM327 commands
- Subscribe, to receive the response

## AT Commands
Keep in mind that all these codes start with 'AT" and require '\r' at the end.

| Purpose | Code | 
| :--- | :--- | 
| Device Description | I |
| Protocol Information | DP |

E.g. A write is done with 'ATI\r', the response will be '410C1F2E\r' followed by '\r>'

More can be found on [Sparkfun](https://cdn.sparkfun.com/assets/4/e/5/0/2/ELM327_AT_Commands.pdf).

## OBD2 Commands
In order to get current date a call needs to be made to 'Service 01'. Followed by the PID and all require '\r' at the end.

| Purpose | Code | Bytes | Min value | Max value |
| :--- | :--- | :--- |  :--- | :--- | 
| Engine Speed | 0C | 2 | 0 |16,383.75|
| Vechicle Speed | 0D | 1 | 0 | 255 |
| Fuel Level | 2F | 1 | 0 | 100|

E.g. A write is done with '010C\r', the response will be '410C1F2E\r' followed by '\r>'

More can be found on [Wikipedia](https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01).
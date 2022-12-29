# About

Many Mettler Toledo scales provide RS232 interfaces for control and measure purposes.

Metosi is single header C library covering basic features of <b>Me</b>ttler <b>To</b>ledos <b>SI</b>CS protocol.

See included hello_scale.ino example code on how to run metosi with ESP32-Arduino.

Metosi is a private project and not affiliated with Mettler Toledo. Metosi is provided as is. 

I do not take responsibility for any damage that might happen as a result of using this code.

# Setup


<b>Attention:</b> You might need to use a logic-level shifter to connect scale and µC. 
 A minimal setup consists of a voltage divider between scales Tx and µC Rx.

1. #include "metosi.h"

2. Implement the following helper method needed by metosi to communicate, char per char, with the scale:

-  `void metosi_send_char(const char* msg)`

3. provide the following callbacks to run when metosi receives a response including meaurement, tare, zero, serialnumber or busy information:

- `void metosi_measurement_cb(uint8_t rv, float weight, char* unit)`
- `void metosi_tare_cb(int rv)`
- `void metosi_zero_cb)(int rv)`
- `void metosi_serialnumber_cb (uint32_t SN)`
- `void metosi_scale_busy_cb(void)`


# Usage

call `metosi_read(char c)` with every single char you received from your scale.

Send commands to scale with `scale_cmd(char* cmd)`.

Implemented commands are:

```
SCALE_TARE_CMD
SCALE_MEASURE_CMD
SCALE_DISPLAY_CMD
SCALE_DISPLAY_WEIGHT_CMD
SCALE_ZERO_CMD
SCALE_RESET_CMD
SCALE_POWER_OFF_CMD
SCALE_POWER_ON_CMD

```
example of how to request a single measurement:
- call `scale_cmd(SCALE_MEASURE_CMD);`
- wait for scale to reply with measurement and metosi to execute  `metosi_measurement_cb(uint8_t rv, float weight, char* unit)`

The scale might not be able to handle multiple querys one after the other. Always wait for a certain amount of milliseconds or (better) await response before sending next command.

## Special commands
`scale_display(const char* text)` print text to the scales screen. 

<img src="https://raw.githubusercontent.com/andrejanowicz/metosi/main/hello.jpg" width="640"></br><br>

To switch back to standard weight display call `scale_cmd(SCALE_DISPLAY_WEIGHT_CMD);`

 

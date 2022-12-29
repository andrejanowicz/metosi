# About

Many Mettler Toledo scales provide a RS232 interfaces for control and measure purposes.

Metosi is single header C library covering basic features of <b>Me</b>ttler <b>To</b>ledos <b>SI</b>CS protocol.

Metosi is a private project and not affiliated with Mettler Toledo. Metosi is provided as is. I do not take responsibility for any damage that might happen as a result of using this code.

# Setup

<b>Attention:</b> When connecting µC and scale keep in mind that you might need to use a logic-level shifter. At very least a voltage divider on the scales TxD.

include metosi.h 

Implement the following helper methods and callbacks:


-  `void metosi_send_char(const char* msg) // used by metosi to communicate, char per char, with the scale`

The following methods execute when the scale sends a response including meaurement, tare, zero, serialnumber or busy information:

- `void metosi_measurement_cb(uint8_t rv, float weight, char* unit)`
- `void metosi_tare_cb(int rv)`
- `void metosi_zero_cb)(int rv)`
- `void metosi_serialnumber_cb (uint32_t SN)`
- `void metosi_scale_busy_cb(void)`


call metosi_read(char c) with every single char you received from your scale.

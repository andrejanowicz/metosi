# About

Metosi is single header library implementing basic functionality of <b>Me</b>ttler <b>To</b>ledos <b>SI</b>CS protocol, written in C.
Many Mettler Toledo scales provide an UART interface for control and measure purposes. With metosi you can easily interact with your scale.
Metosi is a private project and not affiliated with Mettler Toledo. Metosi is provided as is. I do not take responsibility for any damage that might happen as a result of using this code.

# Setup

include metosi.h in your project.

Implement the following helper methods and callbacks:

- void metosi_send_char(const char* msg)
- void metosi_measurement_cb(uint8_t rv, float weight, char* unit)
- void metosi_tare_cb(int rv)
- void metosi_zero_cb)(int rv)
- void metosi_serialnumber_cb (uint32_t SN)
- void metosi_scale_busy_cb(void)


call metosi_read(char c) with every new character you have received from the scale.

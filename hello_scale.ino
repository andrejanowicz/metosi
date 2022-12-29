#include "metosi.h"

// provide a method to send a single char to scale
void send_char(const char* msg){
  if(msg == NULL) return;
  Serial2.print(msg[0]);
}


// receive measurement
void rcv_measurement(uint8_t rv, float weight, char* unit) {
  switch(rv){
    case SCALE_FINE_E:
        Serial.print("scale weight:\t");
        Serial.print(weight);
        Serial.println(unit);
        break;
    case SCALE_HIGH_E:
        Serial.println("Scale weight is too high");
        break;
    case SCALE_LOW_E:
        Serial.println("Scale weight is too low");
        break;
    case SCALE_COM_E:
        Serial.println("Scale communication error");
        break;
    default:
        Serial.print("unknown error:\t");
        Serial.println(rv);
        break;
  }
}

// receive tare response
void rcv_tare(int rv) {
  switch(rv){
    case SCALE_FINE_E:
        Serial.print("Scale tared");
        break;
    case SCALE_TARE_HIGH_E:
        Serial.println("Scale weight is too high to tare");
        break;
    case SCALE_TARE_LOW_E:
        Serial.println("Scale weight is too low to tare");
        break;
    case SCALE_TARE_E:
        Serial.println("Scale tare error");
        break;
    default:
        Serial.print("Unknown error:\t");
        Serial.println(rv);
        break;
    }
}


// receive zero response
void rcv_zero(int rv) {
  switch(rv){
    case SCALE_FINE_E:
        Serial.print("Scale tared");
        break;
    case SCALE_ZERO_HIGH_E:
        Serial.println("Scale weight is too high to tare");
        break;
    case SCALE_ZERO_LOW_E:
        Serial.println("Scale weight is too low to tare");
        break;
    case SCALE_ZERO_E:
        Serial.println("Scale tare error");
        break;
    default:
        Serial.print("Unknown error:\t");
        Serial.println(rv);
        break;
    }
}

// receive serialnumber
void rcv_serialnumber(uint32_t SN) {
  Serial.print("scale SN:\t");
  Serial.println(SN);
}


// receive when scale reports busy
void rcv_busy(){
  Serial.println("Scale reports busy/off");
}


void setup() {
  
  // set metosi callbacks and helper functions
  metosi_send_char = &send_char; 
  metosi_tare_cb = &rcv_tare;
  metosi_zero_cb = &rcv_zero;
  metosi_measurement_cb = &rcv_measurement;
  metosi_serialnumber_cb = &rcv_serialnumber;
  metosi_scale_busy_cb = &rcv_busy;
  
  const uint16_t SCALE_BAUDRATE = 19200;
  const uint8_t RXD = 13;
  const uint8_t TXD = 14;
  const bool INVERT = true;

  Serial.begin(115200);
  Serial2.begin(SCALE_BAUDRATE, SERIAL_8N1, RXD, TXD, INVERT);


  // after UART initilization, first byte sent shows framing error.
  // send two commands to sync with scale
  scale_cmd(SCALE_RESET_CMD);
  delay(100);
  scale_cmd(SCALE_RESET_CMD);
  delay(100);
  
  // from here on everything is good.
  scale_display("HELLO");
  delay(1000);
  scale_display("SCALE");
  delay(1000);  
  scale_cmd(SCALE_DISPLAY_WEIGHT_CMD);
  delay(100);
}


void loop() {

  scale_cmd(SCALE_MEASURE_CMD);
  delay(500);

  // read incoming chars from Serial buffer 
  while (Serial2.available()) {

    char c = Serial2.read();
    
    // and feed them to metosi
    metosi_read(c);
  }
}


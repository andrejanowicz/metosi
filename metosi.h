// 
// metosi.h
// github.com/andre.janowicz/metosi/
//
// basic and unofficial 
// Mettler Toledo SICS protocol library
//
// Author: André Janowicz <andre.janowicz@gmail.com>
// License: CC BY-SA Creative Commons Attribution-ShareAlike
//          https://creativecommons.org/licenses/by-sa/4.0/
//

#pragma once
#include <math.h>

#define SCALE_TARE_CMD            "T"
#define SCALE_MEASURE_CMD         "S"
#define SCALE_DISPLAY_CMD         "D"
#define SCALE_DISPLAY_WEIGHT_CMD  "DW"
#define SCALE_ZERO_CMD            "Z"
#define SCALE_RESET_CMD           "@"
#define SCALE_POWER_OFF_CMD       "PWR 0"
#define SCALE_POWER_ON_CMD        "PWR 1"

//  responses
#define SCALE_INIT                "I4"
#define SCALE_POWER_RESPONSE      "PWR A"
#define SCALE_OK                  "EL"
#define SCALE_SYNTAX_EROR         "ES"
#define SCALE_DISPLAY_OK          "D A"
#define SCALE_MEASURE_STABLE      "S S"
#define SCALE_MEASURE_HIGH        "S +"
#define SCALE_MEASURE_LOW         "S -"
#define SCALE_BUSY                "S I"
#define SCALE_ERROR               "E"
#define SCALE_TARE_OKAY           "T S"
#define SCALE_TARE_HIGH           "T +"
#define SCALE_TARE_LOW            "T -"
#define SCALE_TARE_ERROR          "T I"
#define SCALE_ZERO_OK             "Z A"
#define SCALE_ZERO_HIGH           "Z +"
#define SCALE_ZERO_LOW            "Z -"
#define SCALE_ZERO_ERROR          "Z I"

#define LF                        0x0A
#define SPACE                     0x20


// call this method from within your program with every char that is received from UART
void metosi_read(char c);


// provide a method to send a single char to UART for metosi to communicate with scale
void(*metosi_send_char)(const char*) =  NULL;


// callbacks, see example code

// gets called by metosi whenever scale reports a new measurement
void(*metosi_measurement_cb)(uint8_t, float, char*) =  NULL;


// handle scales tare response
void(*metosi_tare_cb)(int) = NULL;


// handle zero response
void(*metosi_zero_cb)(int) = NULL;


// scale sends serialnumber
void(*metosi_serialnumber_cb)(uint32_t) = NULL;


// scale can not handle request and reports busy
void(*metosi_scale_busy_cb)(void) = NULL;


enum scale_errors{
  SCALE_FINE_E,       // scale is doing well
  SCALE_HIGH_E,       // weight is to high
  SCALE_LOW_E,        // weight is too low
  SCALE_INIT_E,       // init didn´t work
  SCALE_COM_E,        // communication or logic error
  SCALE_TARE_E,       // tare didnt work
  SCALE_TARE_LOW_E,   // tare didnt work, weight is too low
  SCALE_TARE_HIGH_E,  // tare didnt work, weight is too high
  SCALE_ZERO_E,       // zero didnt work
  SCALE_ZERO_LOW_E,   // lower limit of zero range exceeded
  SCALE_ZERO_HIGH_E,  // upper limit of zero range exceeded
};

const uint8_t BUFF_SIZE = 64;
char rcv_buff[BUFF_SIZE];
uint16_t buff_index = 0;
float measurement = 0.0f;
uint8_t decimal_shift = 0;
char unit[8] = "";
uint32_t serial_number = 0;


// sends any command to scale
void scale_cmd(char* cmd){

  char c[1] = "";
  int i = 0;

  while(i<strlen(cmd)){
    sprintf(c,"%c",cmd[i]);
    metosi_send_char(c);
    i++;
  }

  metosi_send_char("\r");
  metosi_send_char("\n");
}

// write string to scale display
void scale_display(const char* text){
  
  const uint8_t MAX_LENGTH = 16;
  char message[MAX_LENGTH] = "D \"";
  const char EOL[] = "\"\r\n\0";

  if(strlen(text) + strlen(message) + strlen(EOL) > MAX_LENGTH) return;

  strcat(message, text);
  strcat(message, EOL);

  char c[1] = "";
  int i = 0;

  while(message[i] != '\0'){
    sprintf(c,"%c",message[i]);
    metosi_send_char(c);
    i++;
  }
}

// call with every char received from scales UART
void metosi_read(char c) {

  //receive next char and fill input buffer
  rcv_buff[buff_index++] = c;

  // LF marks datagram complete
  // at this point rcv_buff[] contains complete reply from scale
  if (c == LF) { 
    buff_index = 0;

    // scale did send init string
    if (strncmp(rcv_buff, SCALE_INIT, strlen(SCALE_INIT)) == 0) {

      serial_number = 0;
      int i = 0;

      // skip until first SN digit
      while (rcv_buff[i] != '"') {  
         i++;
      }

      while (rcv_buff[++i] != '"') {
        serial_number *= 10;

        // ASCII to decimal
        serial_number += (rcv_buff[i] - '0');
      }

      metosi_serialnumber_cb(serial_number);
    }

    // scale reports busy state, might be (soft) powered off
    if (strncmp(rcv_buff, SCALE_BUSY, strlen(SCALE_BUSY)) == 0) {
      metosi_scale_busy_cb();
    }

    // scale did send measurement
    if (strncmp(rcv_buff, SCALE_MEASURE_STABLE, strlen(SCALE_MEASURE_STABLE)) == 0) {
      
      // we got a measurement.
      // Datagram looks like this:
      // [S_S_____100.00_g\n]
      // [S_S12345678.12_g\n]

      measurement = 0;
      decimal_shift = 0;

      // move to end the init string S_S:
      int i = strlen(SCALE_MEASURE_STABLE);

      // skip to the first digit
      while (rcv_buff[i] == SPACE) {
        i++;
      }

      // every digit until space delimiter
      while (rcv_buff[i] != SPACE) {
              
        // remember the decimals point position
        if (rcv_buff[i] == '.') {
          decimal_shift = i;
        }

        // skip leading minus and the decimal_shift delimeter
        if (rcv_buff[i] == '-' || rcv_buff[i] == '.') {
          i++;
        }

        measurement *= 10;

        // ASCII to decimal
        measurement += (rcv_buff[i] - '0'); 
        i++;
      }

      // if measurement was negative
      if (memchr(rcv_buff, '-', sizeof(rcv_buff))) {
        measurement *= -1;
      }

      // restore decimal_shift point
      decimal_shift = i-1 - decimal_shift;   
      measurement /= pow(10,decimal_shift);

      // skip to trailing unit
      while (rcv_buff[i] == SPACE) {
        i++;
      }

      // read unit from datagram
      int j = 0;
      while (rcv_buff[i] != LF){
        unit[j++] = rcv_buff[i++];
      }

      // terminate with 0 byte
      unit[j+1] = '\0';

      // return values to callback function
      metosi_measurement_cb(SCALE_FINE_E, measurement, unit);
    }

    // weight is too high
    if (strncmp(rcv_buff, SCALE_MEASURE_HIGH, strlen(SCALE_MEASURE_HIGH)) == 0) {
      metosi_measurement_cb(SCALE_HIGH_E, 0, NULL);
    }
    
    // weight is too low
    if (strncmp(rcv_buff, SCALE_MEASURE_LOW, strlen(SCALE_MEASURE_LOW)) == 0) {
      metosi_measurement_cb(SCALE_LOW_E, 0, NULL);
    }

    // generic scale error
    if (strncmp(rcv_buff, SCALE_ERROR, strlen(SCALE_ERROR)) == 0) {
      metosi_measurement_cb(SCALE_COM_E, 0, NULL);
    }

    // tare okay
    if (strncmp(rcv_buff, SCALE_TARE_OKAY, strlen(SCALE_TARE_OKAY)) == 0) {
      metosi_tare_cb(SCALE_FINE_E);
    }

    // tare weight too high
    if (strncmp(rcv_buff, SCALE_TARE_HIGH, strlen(SCALE_TARE_HIGH)) == 0) {
      metosi_tare_cb(SCALE_TARE_HIGH_E);
    }
    
    // tare weight too low  
    if (strncmp(rcv_buff, SCALE_TARE_LOW, strlen(SCALE_TARE_LOW)) == 0) {
      metosi_tare_cb(SCALE_TARE_HIGH_E);
    }

    // generic tare error
    if (strncmp(rcv_buff, SCALE_TARE_ERROR, strlen(SCALE_TARE_ERROR)) == 0) {
      metosi_tare_cb(SCALE_TARE_E);
    }

    // zero was succesful
    if (strncmp(rcv_buff, SCALE_ZERO_OK, strlen(SCALE_ZERO_OK)) == 0) {
      metosi_zero_cb(SCALE_FINE_E);
    }
    
    // upper limit of zero setting range exceeded.
    if (strncmp(rcv_buff, SCALE_ZERO_HIGH, strlen(SCALE_ZERO_HIGH)) == 0) {
      metosi_zero_cb(SCALE_ZERO_HIGH_E);
    }

    // lower limit of zero setting range exceeded.
    if (strncmp(rcv_buff, SCALE_ZERO_LOW, strlen(SCALE_ZERO_LOW)) == 0) {
      metosi_zero_cb(SCALE_ZERO_LOW_E);
    }

    // zero setting not performed (balance is currently executing
    // another command, e.g. taring, or timeout as stability was not
    // reached)
    if (strncmp(rcv_buff, SCALE_ZERO_ERROR, strlen(SCALE_ZERO_ERROR)) == 0) {
      metosi_zero_cb(SCALE_ZERO_E);
    }

    // empty rcv_buff[] & unit[]
    memset(&rcv_buff[0], 0, sizeof(rcv_buff));
    memset(&unit[0], 0, sizeof(unit));
  }
}

/*
*  uc_master
Write by: Felipe Hooper
Electronic Engineer
*/

#include <avr/wdt.h>
#include "mlibrary.h"


void setup() {
  wdt_disable();

  Serial.begin(9600);
  mySerial.begin(9600);
  mixer1.begin(9600);

  message.reserve(65);

  DDRB = DDRB | (1<<PB0) | (1<<PB5);
  PORTB = (0<<PB0) | (1<<PB5);

  wdt_enable(WDTO_8S);
}


void loop() {
  if ( stringComplete  ) {
      if ( validate() ) {
          PORTB = 1<<PB0;

          switch ( message[0] ) {
              case 'r':
                hamilton_sensors();
                daqmx();
                control_ph();
                control_temp();
                broadcast_setpoint(0);
                break;

              case 'w':
                setpoint();
                control_ph();
                control_temp();
                broadcast_setpoint(1);
                break;

              case 'c':
                sensor_calibrate();
                break;

              case 'u':
                actuador_umbral();
                break;


              default:
                break;
          }

          PORTB = 0<<PB0;
      }
      else {
        Serial.println("bad validate");
      }

    clean_strings();
    wdt_reset();
  }
}

/*
*  uc_master
*/

#include <avr/wdt.h>
#include "mlibrary.h"

void setup() {
  wdt_disable();

  Serial.begin(9600);
  mySerial.begin(9600);

  message.reserve(65);

  DDRB = DDRB | (1<<PB0);
  PORTB = 0<<PB0;

  wdt_enable(WDTO_2S);
}


void loop() {

  if ( stringComplete  ) {
      if ( validate() ) {

          switch ( message[0] ) {
              case 'r':
                hamilton_sensors();
                daqmx();
                broadcast_setpoint(0);
                break;

              case 'w':
                setpoint();
                broadcast_setpoint(1);
                break;

              case 'c':
                calibrate();
                break;

              default:
                break;
          }
          clean_strings();
      }
      else {
        Serial.println("bad");
      }
  }

  wdt_reset();
}

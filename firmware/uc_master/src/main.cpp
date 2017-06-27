/*
*  uc_master
*/

#include <avr/wdt.h>
#include "mlibrary.h"
#include "PID_v1.h"


void setup() {
  wdt_disable();

  Serial.begin(9600);
  mySerial.begin(9600);

  message.reserve(65);

  DDRB = DDRB | (1<<PB0) | (1<<PB5);
  PORTB = (0<<PB0) | (1<<PB5);

  TEMP_PID.SetMode(AUTOMATIC);
  TEMP_PID.SetOutputLimits(0,+umbral_temp);
  TEMP_PID.SetSampleTime(Ts);

  PH_PID.SetMode(AUTOMATIC);
  PH_PID.SetOutputLimits(-umbral_a,+umbral_a);
  PH_PID.SetSampleTime(Ts);

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
                pid_ph();
                pid_temp();
                broadcast_setpoint(0);
                break;

              case 'w':
                setpoint();
                pid_ph();
                pid_temp();
                broadcast_setpoint(1);
                break;

              case 'c':
                sensor_calibrate();
                break;

              case 'u':
                actuador_umbral();
                break;

              case 't':
                pid_tuning();
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
  //PID_PH();  //control ph
  //wdt_reset();
}

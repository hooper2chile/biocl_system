/*
*  uc_master
*  Writed by: Felipe Hooper
*  Electronic Engineer
*/

#include <avr/wdt.h>
#include "mlibrary.h"


void setup() {
  wdt_disable();

  Serial.begin(9600);
  ads1.begin();

  mySerial.begin(9600);
  mixer1.begin(9600);

  message.reserve(65);

  DDRB = DDRB | (1<<PB0) | (1<<PB5);
  PORTB = (0<<PB0) | (1<<PB5);

//analogReference(EXTERNAL);
//                                                              ADS1015  ADS1115
//                                                              -------  -------
//ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  ads1.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads2.setGain(GAIN_ONE);
//ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
//ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
//ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
//ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

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
		daqmx();
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

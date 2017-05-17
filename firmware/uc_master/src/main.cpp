/*
*  uc_master
*/

#include <avr/wdt.h>
#include <mylibrary_master.h>

void hamilton_sensor() {
 Iph    = 0;
 Iod    = 0;
 Itemp1 = 0;
 Itemp2 = 0;

 for (int i = 1; i <= N; i++) {
   Iph    += analogRead(SENSOR_PH);
   Iod    += analogRead(SENSOR_OD);
   Itemp1 += analogRead(SENSOR_TEMP1);
   Itemp2 += analogRead(SENSOR_TEMP2);
   delayMicroseconds(200);
 }

 Iph    =  (K * Iph  );
 Itemp1 =  (K * Itemp1);

 Iod    =  (K * Iod);
 Itemp2 =  (K * Itemp2);
 //pH   = m1*K*Iph   + n1;
 //Temp = m2*K*Itemp + n2;
 wdt_reset();

 return;
}


void DAQmx() {
  if (stringComplete) {
    PORTB = 1<<PB0;
    //Read case
    if (message[0] == 'r') {
      hamilton_sensor();

      Byte0 = Itemp1;
      Byte1 = Iod;
      Byte2 = Itemp2;

      dtostrf(Byte0, 7, 2, cByte0);
      dtostrf(Byte1, 7, 2, cByte1);
      dtostrf(Byte2, 7, 2, cByte2);

      //tx measure
      Serial.print(cByte0);  Serial.print("\t");
      Serial.print(cByte1);  Serial.print("\t");
      Serial.print(cByte2);  Serial.print("\t");
      Serial.print("\n");

      //lcd.clear();
      //lcd.print(message);
      //Re-transmition of last command to slave micro controller in each reading
      mySerial.print(new_write);

      clean_strings();
    }

    //Re-transmition write case
    else if (validate_write()) {
      Serial.println("good");

      //lcd.clear();
      //lcd.print(message);

      //Re-transmition of NEW command to slave micro controller
      new_write = "";
      new_write = message;
      mySerial.print(new_write);

      clean_strings();
    }


    else {
      Serial.println("bad");
      clean_strings();
    }
  }
  PORTB = 0<<PB0;
  wdt_reset();
}

void setup() {
  wdt_disable();
  //lcd init
  //lcd.begin(16,2);
  Serial.begin(9600);
  mySerial.begin(9600);

  //reserve 32 bytes for the message:
  message.reserve(65);

  DDRB = 0b00000001;
  PORTB = 0<<PB0;

  wdt_enable(WDTO_2S);
}

void loop() {
  DAQmx();

}

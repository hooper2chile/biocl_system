/*
  uc_master
*/

#include "Arduino.h"
#include <avr/wdt.h>

#include "SoftwareSerial.h"
SoftwareSerial mySerial(2, 3); //RX(Digital2), TX(Digital3) Software serial port.

//#include <LiquidCrystal.h>
//LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

#define  INT(x)          (x-48)  //ascii convertion
#define iINT(x)          (x+48)  //inverse ascii convertion

String  message     = "";
String  new_write   = "";
boolean stringComplete = false;  // whether the string is complete

String  ph_var      = ""; String  ph_set      = "";
String  feed_var    = ""; String  feed_set    = "";
String  unload_var  = ""; String  unload_set  = "";
String  mix_var     = ""; String  mix_set     = "";
String  temp_var    = ""; String  temp_set    = "";


//RESET SETUP
char rst1 = 1;  char rst2 = 1;  char rst3 = 1;
char rst4 = 1;  char rst5 = 1;  char rst6 = 1;

//DIRECTION SETUP
char dir1 = 1;  char dir2 = 1;  char dir3 = 1;
char dir4 = 1;  char dir5 = 1;  char dir6 = 1;


float   myphset  = 0;
uint8_t myfeed   = 0;
uint8_t myunload = 0;
uint8_t mymix    = 0;
uint8_t mytemp   = 0;

// for incoming serial data
float Byte0 = 0;  char cByte0[15] = "";
float Byte1 = 0;  char cByte1[15] = "";
float Byte2 = 0;  char cByte2[15] = "";


// Sensors
const int SENSOR_PH    = A0;  // Input pin for measuring Vout
const int SENSOR_TEMP1 = A7;
const int VOLTAGE_REF  = 5;  // Reference voltage for analog read
const int RS = 10;          // Shunt resistor value (in ohms)
const int N  = 250;

float Iph;
float Itemp;

float pH;        //   ph = 0.896*IpH   - 3.52
float Temp;      // Temp = 5.31*Itemp - 42.95;
float aer;

#define mA 1000.0
#define K  ( mA * ( ( (VOLTAGE_REF/1023.0) / ( 10.0 * RS ) ) / N ) )



void clean_strings() {
  //clean strings
  stringComplete = false;  message = "";
}


//for hardware serial
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    message += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}


//message format write values: wph14.0feed100unload100mix1500temp100rst111111dir111111
int validate_write() {

  if (
    message[0] == 'w'                     &&
    message.length() == 56                &&

    message.substring(1, 3)   == "ph"     &&
    message.substring(7, 11)  == "feed"   &&
    message.substring(14, 20) == "unload" &&
    message.substring(23, 26) == "mix"    &&
    message.substring(30, 34) == "temp"   &&
    message.substring(37, 40) == "rst"    &&
    message.substring(46, 49) == "dir"    &&

    //ph number
    ( message.substring(3, 7).toFloat() >= 0    ) &&
    ( message.substring(3, 7).toFloat() <= 14.0 ) &&

    //feed number
    ( message.substring(11, 14).toInt() >= 0   ) &&
    ( message.substring(11, 14).toInt() <= 100 ) &&

    //unload number
    ( message.substring(20, 23).toInt() >= 0   ) &&
    ( message.substring(20, 23).toInt() <= 100 ) &&

    //mix number
    ( message.substring(26, 30).toInt() >= 0   ) &&
    ( message.substring(26, 30).toInt() <= 1500) &&

    //temp number
    ( message.substring(34, 37).toInt() >= 0   ) &&
    ( message.substring(34, 37).toInt() <= 100 ) &&

    //rst bits
    ( message[40] == iINT(1) || message[40] == iINT(0) ) &&
    ( message[41] == iINT(1) || message[41] == iINT(0) ) &&
    ( message[42] == iINT(1) || message[42] == iINT(0) ) &&
    ( message[43] == iINT(1) || message[43] == iINT(0) ) &&
    ( message[44] == iINT(1) || message[44] == iINT(0) ) &&
    ( message[45] == iINT(1) || message[45] == iINT(0) ) &&

    //dir bits

    ( message[49] == iINT(1) || message[49] == iINT(0) ) &&
    ( message[50] == iINT(1) || message[50] == iINT(0) ) &&
    ( message[51] == iINT(1) || message[51] == iINT(0) ) &&
    ( message[52] == iINT(1) || message[52] == iINT(0) ) &&
    ( message[53] == iINT(1) || message[53] == iINT(0) ) &&
    ( message[54] == iINT(1) || message[54] == iINT(0) )
  )
  return 1;

  else
    return 0;
}

//desmenuza el string de comandos
void crumble() {
  //Serial.println("good");

  ph_var = message.substring(1, 3);
  ph_set = message.substring(3, 7);

  feed_var = message.substring(7, 11);
  feed_set = message.substring(11, 14);

  unload_var = message.substring(14, 20);
  unload_set = message.substring(20, 23);

  mix_var = message.substring(23, 26);
  mix_set = message.substring(26, 30);

  temp_var = message.substring(30, 34);
  temp_set = message.substring(34, 37);


  //setting setpoints
  myphset  = ph_set.toFloat();
  myfeed   = feed_set.toInt();
  myunload = unload_set.toInt();
  mymix    = mix_set.toInt();
  mytemp   = temp_set.toInt();

  //setting rst
  rst1 = INT(message[40]);  rst2 = INT(message[41]);  rst3 = INT(message[42]);
  rst4 = INT(message[43]);  rst5 = INT(message[44]);  rst6 = INT(message[45]);

  //setting dir
  dir1 = INT(message[49]);  dir2 = INT(message[50]);  dir3 = INT(message[51]);
  dir4 = INT(message[52]);  dir5 = INT(message[53]);  dir6 = INT(message[54]);

  return;
}



float m1 = +0.896; float m2 = +5.31;
float n1 = -3.52;  float n2 = -42.95;

void easyferm_sensor() {
 Iph   = 0;
 Itemp = 0;

 for (int i = 1; i <= N; i++) {
   Iph   += analogRead(SENSOR_PH);
   Itemp += analogRead(SENSOR_TEMP1);
   delayMicroseconds(200);
 }
 //Iph   = (K * Iph);
 //Itemp = (K * Itemp);

 pH   = m1*K*Iph   + n1;
 Temp = m2*K*Itemp + n2;

 wdt_reset();
 return;
}


void DAQmx() {
  if (stringComplete) {
    PORTB = 1<<PB0;
    //Read case
    if (message[0] == 'r') {
      easyferm_sensor();

      Byte0 = pH;
      Byte1 = analogRead(1) / 10.23;
      Byte2 = Temp;

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

  wdt_reset();
}

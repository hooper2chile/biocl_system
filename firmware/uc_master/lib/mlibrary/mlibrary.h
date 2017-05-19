#include "Arduino.h"

#include "SoftwareSerial.h"
SoftwareSerial mySerial(2, 3); //RX(Digital2), TX(Digital3) Software serial port.

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
float Byte3 = 0;  char cByte3[15] = "";
float Byte4 = 0;  char cByte4[15] = "";
float Byte5 = 0;  char cByte5[15] = "";

// Sensors
const int SENSOR_PH    = A0;  // Input pin for measuring Vout
const int SENSOR_TEMP1 = A1;
const int SENSOR_TEMP2 = A2;
const int SENSOR_OD    = A3;

const int VOLTAGE_REF  = 5;  // Reference voltage for analog read
const int RS = 10;          // Shunt resistor value (in ohms)
const int N  = 250;

float Iph;
float Iod;
float Itemp1;
float Itemp2;

float pH;         //   ph = 0.75*IpH   - 3.5
float oD;
float Temp1;      // Temp = 5.31*Itemp - 42.95;
float Temp2;

//pH=:(m1,n1)
float m1 = +0.75;
float n1 = -3.5;

//oD=:(m2,n2)
float m2 = +5.31;
float n2 = -42.95;

//Temp1=:(m3,n3)
float m3 = 0;
float n3 = 0;

#define mA 1000.0
#define K  ( mA * ( ( (VOLTAGE_REF/1023.0) / ( 10.0 * RS ) ) / N ) )



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


void hamilton_sensors() {

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

  return;
}


void daqmx() {

  Byte0 = pH;
  Byte1 = oD;
  Byte2 = Itemp1;
  Byte3 = Iph;
  Byte4 = Iod;
  Byte5 = Itemp1-Itemp2;

  dtostrf(Byte0, 7, 2, cByte0);
  dtostrf(Byte1, 7, 2, cByte1);
  dtostrf(Byte2, 7, 2, cByte2);
  dtostrf(Byte3, 7, 2, cByte3);
  dtostrf(Byte4, 7, 2, cByte4);
  dtostrf(Byte5, 7, 2, cByte5);

  //tx of measures
  Serial.print(cByte0);  Serial.print("\t");
  Serial.print(cByte1);  Serial.print("\t");
  Serial.print(cByte2);  Serial.print("\t");
  Serial.print(cByte3);  Serial.print("\t");
  Serial.print(cByte4);  Serial.print("\t");
  Serial.print(cByte5);  Serial.print("\t");
  Serial.print("\n");

  return;
}


void calibrate(){
  //calibrate function for "message"
  m1 = message.substring(1,6).toFloat();
  m2 = message.substring(6,11).toFloat();
  m3 = message.substring(11,16).toFloat();

  n1 = message.substring(17,22).toFloat();
  n2 = message.substring(22,27).toFloat();
  n3 = message.substring(27,32).toFloat();

  pH    = m1 * Iph    + n1;
  oD    = m2 * Iod    + n2;
  Temp1 = m3 * Itemp1 + n3;

  return;
}


void setpoint() {
  //eventualmente, aca hay que programar el mezclador y usar crumble() para obtener el dato
  Serial.println("good");

  return;
}
void broadcast_setpoint(uint8_t select) {
  //Re-transmition of NEW command to slave micro controller
  switch (select) {
    case 0: //only re-tx.
      mySerial.print(new_write);
      break;

    case 1: //update command and re-tx.
      new_write = "";
      new_write = message;
      mySerial.print(new_write);
      break;

    default:
      break;
  }

  return;
}
void clean_strings() {
  //clean strings
  stringComplete = false;  message = "";
}


int validate() {
PORTB = 1<<PB0;
//message format write values: wph14.0feed100unload100mix1500temp100rst111111dir111111
    if (  message[0] == 'w'       /*            &&
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
          ( message[54] == iINT(1) || message[54] == iINT(0) )*/
        )
        { return 1; }

      else if ( message[0] == 'c' && message[16] == 'a' && message[32] == 'l' )
          return 1;

      else if ( message[0] == 'r' )
          return 1;

      else
          return 0;
PORTB = 0<<PB0;
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

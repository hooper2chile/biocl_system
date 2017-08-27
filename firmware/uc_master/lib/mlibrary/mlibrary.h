#include "Arduino.h"

#include "SoftwareSerial.h"
SoftwareSerial mySerial(2, 3);  //RX(Digital2), TX(Digital3) Software serial port.
SoftwareSerial mixer1(4, 5); //for control in mezclador


#define  INT(x)   (x-48)  //ascii convertion
#define iINT(x)   (x+48)  //inverse ascii convertion

#define SPEED_MIN 2.0
#define SPEED_MAX 150     //[RPM]
#define TEMP_MAX  130     //[ºC]

//#define Ts        1000     //1000ms

#define Gap_temp0 0.5
#define Gap_temp1 1.0       //1ºC
#define Gap_temp2 2.0
#define Gap_temp3 3.0
#define Gap_temp4 5.0

#define Gap_pH_0  0.05
#define Gap_pH_1  0.10     // 0.1 (pH)
#define Gap_pH_2  0.50
#define Gap_pH_3  0.75
#define Gap_pH_4  1.00
#define Gap_pH_5  2.00


String  message     = "";  String  new_write   = "";  String  new_write0   = "";

boolean stringComplete = false;  // whether the string is complete

String  ph_var      = "";   String  ph_set      = "";
String  feed_var    = "";   String  feed_set    = "";
String  unload_var  = "";   String  unload_set  = "";
String  mix_var     = "";   String  mix_set     = "";
String  temp_var    = "";   String  temp_set    = "";


//Re-formatting
String  ph_select = "";
String  uset_temp = "";
String  uset_ph   = "";
String  svar      = "";


//RESET SETUP
char rst1 = 1;  char rst2 = 1;  char rst3 = 1;
char rst4 = 1;  char rst5 = 1;  char rst6 = 1;

//DIRECTION SETUP
char dir1 = 1;  char dir2 = 1;  char dir3 = 1;
char dir4 = 1;  char dir5 = 1;  char dir6 = 1;

float   myphset   = 0;
float   mytempset = 0;

uint8_t myfeed    = 0;
uint8_t myunload  = 0;
uint16_t mymix     = 0;

int i = 0;
int data = 0;
int data_cero = 0;
uint16_t s_rpm_save = 0;

float umbral_a = SPEED_MAX;
float umbral_b = SPEED_MAX;
float umbral_temp = SPEED_MAX;

// for incoming serial data
float Byte0 = 0;  char cByte0[15] = "";  //por que no a 16?
float Byte1 = 0;  char cByte1[15] = "";
float Byte2 = 0;  char cByte2[15] = "";
float Byte3 = 0;  char cByte3[15] = "";
float Byte4 = 0;  char cByte4[15] = "";
float Byte5 = 0;  char cByte5[15] = "";
float Byte6 = 0;  char cByte6[15] = "";

// Sensors
const int SENSOR_PH    = A0;  // Input pin for measuring Vout
const int SENSOR_TEMP1 = A1;
const int SENSOR_TEMP2 = A2;
const int SENSOR_OD    = A3;

const int VOLTAGE_REF  = 5;  // Reference voltage for analog read
const int RS = 10;          // Shunt resistor value (in ohms)
const int N  = 200; //500

//calibrate function()
char  var = '0';
float m = 0;
float n = 0;

//pH=:(m0,n0)
float m0 = +0.75;
float n0 = -3.5;

//oD=:(m1,n1)
float m1 = +6.02;
float n1 = -20.42;

//Temp1=:(m2,n2)
float m2 = +5.31;
float n2 = -42.95;


float Iph = 0;
float Iod = 0;
float Itemp1 = 0;
float Itemp2 = 0;
                                   //   DEFAULT:
float pH    = m0*Iph    + n0;      //   ph = 0.75*IpH   - 3.5
float oD    = m1*Iod    + n1;
float Temp1 = m2*Itemp1 + n2;      // Temp = 5.31*Itemp - 42.95;
//float Temp2;


//variable for control
float u_temp = 0;
float u_ph = 0;
float dTemp= 0;
float dpH  = 0;




//for sensors 4-20mA
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



//desmenuza el string de comandos
void write_crumble() {
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
  mytempset= temp_set.toFloat();

  myfeed   = feed_set.toInt();
  myunload = unload_set.toInt();
  mymix    = mix_set.toInt();


  //setting rst
  rst1 = INT(message[40]);  rst2 = INT(message[41]);  rst3 = INT(message[42]);
  rst4 = INT(message[43]);  rst5 = INT(message[44]);  rst6 = INT(message[45]);

  //setting dir
  dir1 = INT(message[49]);  dir2 = INT(message[50]);  dir3 = INT(message[51]);
  dir4 = INT(message[52]);  dir5 = INT(message[53]);  dir6 = INT(message[54]);

  return;
}




//c2+00.75-03.50e   (0=>ph) (1=>od) (2=>temp)
void sensor_calibrate(){
  //calibrate function for "message"
  var = message[1];
  m   = message.substring(2,8 ).toFloat();
  n   = message.substring(8,14).toFloat();

  switch (var) {
    case '0': //pH case for calibration
      m0 = m;
      n0 = n;
      break;

    case '1': //Oxigen disolve case for calibration
      m1 = m;
      n1 = n;
      break;

    case '2': //Temperature case for calibration
      m2 = m;
      n2 = n;
      break;

    default:
      break;
  }

  Serial.println("Sensor calibrated");
  return;
}



//modifica los umbrales de cualquiera de los dos actuadores
void actuador_umbral(){
  //setting threshold ph: u1a160b141e
  if ( message[1] == '1' ) {

    umbral_a = 0; umbral_b = 0;
    umbral_a = message.substring(3,6).toFloat();
    umbral_b = message.substring(7,10).toFloat();

    if ( umbral_a <= SPEED_MIN )
      umbral_a = SPEED_MIN;
    else if ( umbral_a >= SPEED_MAX )
      umbral_a = SPEED_MAX;

    if ( umbral_b <= SPEED_MIN )
      umbral_b = SPEED_MIN;
    else if ( umbral_b >= SPEED_MAX )
      umbral_b = SPEED_MAX;

  }
  //setting threshold temp: u2t011e
  else if ( message[1] == '2' ) {

    umbral_temp = 0;
    umbral_temp = message.substring(3,6).toFloat();

    if ( umbral_temp <= SPEED_MIN )
      umbral_temp = SPEED_MIN;
    else if ( umbral_temp >= SPEED_MAX)
      umbral_temp = SPEED_MAX;
    else
      umbral_temp = umbral_temp;

  }

  Serial.println( String(umbral_a) + '_' + String(umbral_b) + '_' + String(umbral_temp) );
  return;
}



void hamilton_sensors() {

  Iph    = 0;   Iod    = 0;
  Itemp1 = 0;   Itemp2 = 0;

  for (int i = 1; i <= N; i++) {
     Iph    += analogRead(SENSOR_PH);
     Iod    += analogRead(SENSOR_OD);
     Itemp1 += analogRead(SENSOR_TEMP1);
     Itemp2 += analogRead(SENSOR_TEMP2);
     delayMicroseconds(200);
  }

  Iph    = (K * Iph  );
  Itemp1 = (K * Itemp1);

  Iod    = (K * Iod);
  Itemp2 = (K * Itemp2);

  //Update measures
  pH    = m0 * Iph    + n0;
  oD    = m1 * Iod    + n1;
  Temp1 = m2 * Itemp1 + n2;

  return;
}





void daqmx() {
  //data adquisition measures
  Byte0 = pH;
  Byte1 = oD;
  Byte2 = Temp1;
  Byte3 = Iph;
  Byte4 = Iod;
  Byte5 = Itemp1;
  Byte6 = Itemp2;


  dtostrf(Byte0, 7, 2, cByte0);
  dtostrf(Byte1, 7, 2, cByte1);
  dtostrf(Byte2, 7, 2, cByte2);
  dtostrf(Byte3, 7, 2, cByte3);
  dtostrf(Byte4, 7, 2, cByte4);
  dtostrf(Byte5, 7, 2, cByte5);
  dtostrf(Byte6, 7, 2, cByte6);

  //tx of measures
  Serial.print(cByte0);  Serial.print("\t");
  Serial.print(cByte1);  Serial.print("\t");
  Serial.print(cByte2);  Serial.print("\t");
  Serial.print(cByte3);  Serial.print("\t");
  Serial.print(cByte4);  Serial.print("\t");
  Serial.print(cByte5);  Serial.print("\t");
  Serial.print(cByte6);  Serial.print("\t");

  //for debug
  Serial.print("__ua="+String(umbral_a)+"__ub="+String(umbral_b)+"__myphset="+String(myphset)+"__pH="+String(pH)+"__dpH="+String(dpH)+"__u_ph="+String(u_ph)+"__ph_select="+String(ph_select));  Serial.print("\t");
  Serial.print("\n");

  return;
}



void control_temp() {
  //for debug
  //mytemp  = 50;

  //touch my delta temp
  dTemp = mytempset - Temp1;

  if ( dTemp > 0.0 ) {
    if ( dTemp <= Gap_temp1 )
      u_temp = 0.20*umbral_temp;

    else if ( dTemp <= Gap_temp2 )
      u_temp = 0.35*umbral_temp; //50%

    else if ( dTemp <= Gap_temp3 )
      u_temp = 0.50*umbral_temp; //75%

    else if ( dTemp <= Gap_temp4 )
      u_temp = 0.75*umbral_temp; //100%

    else if ( dTemp > Gap_temp4 )
      u_temp = umbral_temp;
  }
  //dTemp < 0 => speed min in actuador temp
  else if ( dTemp <= 0.0 )
    u_temp = SPEED_MIN;


  return;
}


void control_ph() {
  //for debug
  //myphset = 7.0;

  //touch my delta ph
  dpH = myphset - pH;

  // Escenario en que se debe aplicar acido.
  if ( dpH > 0.0 ) {
    if ( dpH <= Gap_pH_0 ) //5% ó OFF según sí el 5% de umbral_a sea < 1
      u_ph = 0.05 * umbral_b;

    else if ( dpH <= Gap_pH_1 )
      u_ph = 0.1 * umbral_b;  //10%

    else if ( dpH <= Gap_pH_2 )
      u_ph = 0.2 * umbral_b;  //20%

    else if ( dpH <= Gap_pH_3 )
      u_ph = 0.3 * umbral_b;  //30%

    else if ( dpH <= Gap_pH_4 )
      u_ph = 0.5 * umbral_b; //50%

    else if ( dpH <= Gap_pH_5 )
      u_ph = 0.75 * umbral_b;//75%

    else if ( dpH > Gap_pH_5 )
      u_ph = umbral_b;       //100%

    ph_select = "b";  //=> Acido
    }

  // Escenario en que se debe aplicar base.
  else if ( dpH <= 0.0 ) {
    if ( dpH >= -Gap_pH_0 )
    u_ph = 0.05 * umbral_a;   //5%

    else if ( dpH >= -Gap_pH_1 )
      u_ph = 0.1 * umbral_a;  //10%

    else if ( dpH >= -Gap_pH_2 )
      u_ph = 0.2 * umbral_a;  //20%

    else if ( dpH >= -Gap_pH_3 )
      u_ph = 0.3 * umbral_a;  //30%

    else if ( dpH >= -Gap_pH_4 )
      u_ph = 0.5 * umbral_a;  //50%

    else if ( dpH >= -Gap_pH_5 )
      u_ph = 0.75 * umbral_a; //75%

    else if ( dpH < -Gap_pH_5 )
      u_ph = umbral_a;        //100%

    ph_select = "a";  //=> Básico
  }

  else {
    u_ph = 0;
    ph_select = "N";  //no hacer nada
  }

  return;
}




void Motor_set_RPM(int high, int low)
{
  int checksum = (177 + high + low) & 0xff;

  mixer1.write(254);
  delay(100);
  mixer1.write(177);
  delay(100);
  mixer1.write(high);
  delay(100);
  mixer1.write(low);
  delay(100);
  mixer1.write(data_cero);
  delay(100);
  mixer1.write(checksum);
}
//254 160 0 0 0 160       254 160 0 0 0 160     254 177 0 0 0 177       254 177 0 0 0 177      254 177 0 d 0 21      254 177
void Motor_conectar()
{
  delay(100);
  mixer1.write(254);
  delay(100);
  mixer1.write(160);
  delay(100);
  data = 0;
  mixer1.write(data);  // data = 0
  delay(100);
  mixer1.write(data);
  delay(100);
  mixer1.write(data);
  delay(100);
  mixer1.write(160);
}

void agitador(uint16_t s_rpm, uint8_t rst) {
    if( !rst2 ) {
      if ( s_rpm_save != s_rpm ) {
        s_rpm_save = s_rpm;
        int rpm_h = (s_rpm >> 8) & 0xff;
        int rpm_l = s_rpm & 0xff;

        while ( i <= 1 ) {
           Motor_conectar();
           Motor_set_RPM(rpm_h, rpm_l);
          i++;
        }
        i = 0;
      }
    }
    else if ( rst ) {
      data_cero = 0;
      s_rpm_save = data_cero;
      int rpm_h = (data_cero >> 8) & 0xff;
      int rpm_l = data_cero & 0xff;

      while ( i <= 1 ) {
         Motor_conectar();
         Motor_set_RPM(rpm_h, rpm_l);
        i++;
      }
      i = 0;
    }
}


void setpoint() {
  //acá se leen los nuevos setpoint para los lazos de control
  write_crumble();

  //aca hay que programar el mezclador y usar crumble() para obtener el dato
  agitador(mymix,rst2);


  Serial.println("good setpoint");
  return;
}


//function for transform numbers to string format of message
void format_message(int var) {
  //reset to svar string
  svar = "";

  if (var < 10)
    svar = "00"+ String(var);

  else if (var < 100)
    svar = "0" + String(var);

  else
    svar = String(var);

  return;
}
//Re-transmition commands to slave micro controller
void broadcast_setpoint(uint8_t select) {

  //se prepara el setpoint para el renvio hacia uc slave.
  format_message(u_temp);
  uset_temp = svar; //string variable for control: uset_temp

  format_message(u_ph);
  uset_ph = ph_select + svar; //add strings for ph control: uset_ph


  switch (select) {
    case 0: //only re-tx and update pid uset's.
      new_write0 = "";
      new_write0 = new_write.substring(0,3) + uset_ph + new_write.substring(7,34) + uset_temp + new_write.substring(37,55) + "\n";
      mySerial.print(new_write0);
      break;

    case 1: //update command and re-tx.
      new_write = "";
      new_write = message.substring(0,3) + uset_ph + message.substring(7,34) + uset_temp + message.substring(37,55) + "\n";
      mySerial.print(new_write);
      break;

    default:
      break;
  }

  return;
}


//wph08.3feed100unload100mix1500temp022rst111111dir111111
//wphb015feed100unload100mix1500temp150rst000000dir111111
void clean_strings() {
  //clean strings
  stringComplete = false;
  message   = "";
  uset_temp = "";
  uset_ph   = "";
  ph_select ="";
}




int validate() {
//message format write values: wph14.0feed100unload100mix1500temp100rst111111dir111111
    // Validate SETPOINT
    if (  message[0] == 'w'                     &&
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
          ( message.substring(11, 14).toInt() >= 0         ) &&
          ( message.substring(11, 14).toInt() <= SPEED_MAX ) &&

          //unload number
          ( message.substring(20, 23).toInt() >= 0         ) &&
          ( message.substring(20, 23).toInt() <= SPEED_MAX ) &&

          //mix number
          ( message.substring(26, 30).toInt() >= 0    ) &&
          ( message.substring(26, 30).toInt() <= 1500 ) &&

          //temp number
          ( message.substring(34, 37).toInt() >= 0        ) &&
          ( message.substring(34, 37).toInt() <= TEMP_MAX ) &&

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
        { return 1; }

      // Validate CALIBRATE
      else if ( message[0]  == 'c' &&
               (message[2]  == '+' || message[2] == '-') &&
               (message[8]  == '+' || message[8] == '-') &&
                message[14] == 'e' &&
                message.substring(3,8 ).toFloat() < 100 &&
                message.substring(9,14).toFloat() < 100
              )
          return 1;

      //Validete umbral actuador ph: u1a001b001e
      else if ( message[0]  == 'u' && message[1] == '1' &&
                message[2]  == 'a' && message[6] == 'b' &&
                message[10] == 'e'
              )
          return 1;

      //Validete umbral actuador temp: u2t003e
      else if ( message[0] == 'u' && message[1] == '2' &&
                message[2] == 't' && message[6] == 'e'
              )
          return 1;

      // Validate READING
      else if ( message[0] == 'r' )
          return 1;

      // NOT VALIDATE
      else
          return 0;
}

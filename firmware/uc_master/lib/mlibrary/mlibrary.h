#include "Arduino.h"

#include "SoftwareSerial.h"
SoftwareSerial mySerial(2, 3); //RX(Digital2), TX(Digital3) Software serial port.


#include "PID_v1.h"
double Setpoint_temp, Input_temp, Output_temp;
double Setpoint_ph, Input_ph, Output_ph;

double Kp_ph=200,   Ki_ph=45,   Kd_ph=0;
double Kp_temp=200, Ki_temp=45, Kd_temp=0;

PID TEMP_PID(&Input_temp, &Output_temp, &Setpoint_temp, Kp_temp, Ki_temp, Kd_temp, DIRECT);
PID   PH_PID(&Input_ph,   &Output_ph,   &Setpoint_ph,   Kp_ph,   Ki_ph,   Kd_ph,   DIRECT);


#define  INT(x)   (x-48)  //ascii convertion
#define iINT(x)   (x+48)  //inverse ascii convertion
#define SPEED_MAX 150
#define TEMP_MAX  130
#define Ts        500 //500ms

String  message     = "";
String  new_write   = "";
boolean stringComplete = false;  // whether the string is complete

String  ph_var      = "";   String  ph_set      = "";
String  feed_var    = "";   String  feed_set    = "";
String  unload_var  = "";   String  unload_set  = "";
String  mix_var     = "";   String  mix_set     = "";
String  temp_var    = "";   String  temp_set    = "";


//Re-formatting
String  ph_select = "";
String  t_temp = "";
String  t_ph   = "";
String  svar   = "";

char pid_x, k_x;


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
char  var='0';
float m=0;
float n=0;

//pH=:(m0,n0)
float m0 = +0.75;
float n0 = -3.5;

//oD=:(m1,n1)
float m1 = 1;
float n1 = 0;

//Temp1=:(m2,n2)
float m2 = +5.31;
float n2 = -42.95;


float Iph = 0;
float Iod = 0;
float Itemp1 = 0;
float Itemp2 = 0;
                                   //   DEFAULT:
float pH    = m0*Iph    + n0;      //   ph = 0.75*IpH   - 3.5
float oD    = m1*Iod    + n1;      // Temp = 5.31*Itemp - 42.95;
float Temp1 = m2*Itemp1 + n2;
//float Temp2;

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




void format_message(int var) {

  //reset to scar string
  svar = "";

  if (var < 10)
    svar = "00"+ String(var);
  else if (var < 100)
    svar = "0" + String(var);
  else
    svar = String(var);

  return;
}



//c2+00.75-03.50e   (0=>ph) (1=>od) (2=>temp)
void sensor_calibrate(){
  //calibrate function for "message"
  var = message[1];
  m   = message.substring(2,8 ).toFloat();
  n   = message.substring(8,14).toFloat();

  switch (var) {
    case '0':
      m0 = m;
      n0 = n;
      break;

    case '1':
      m1 = m;
      n1 = n;
      break;

    case '2':
      m2 = m;
      n2 = n;
      break;

    default:
      break;
  }
  Serial.println("good calibrate");

  return;
}


//t1p9999.9i3333.3d5555.5e
//t0p0200.0i0044.7d0025.5e
void pid_tuning(){
  // message[1] = 1 => pid ph tuning
  if ( message[1] == 1 ){
    Kp_ph = message.substring(3,9).toFloat();
    Ki_ph = message.substring(10,16).toFloat();
    Kd_ph = message.substring(17,23).toFloat();

    PH_PID.SetTunings(Kp_ph, Ki_ph, Kd_ph);
  }
  // message[1] = 2 => pid temp tuning
  else if ( message[1] == 2 ){
    Kp_temp = message.substring(3,9).toFloat();
    Ki_temp = message.substring(10,16).toFloat();
    Kd_temp = message.substring(17,23).toFloat();

    TEMP_PID.SetTunings(Kp_temp, Ki_temp, Kd_temp);
  }

  Serial.println("pid_tuning ready");
};


//modifica los umbrales de cualquiera de los dos actuadores
void actuador_umbral(){
  //setting threshold ph: u1a160b142e
  if ( message[1] == '1' ) {
    uint8_t umbral_a = message.substring(3,6).toInt();
    uint8_t umbral_b = message.substring(7,10).toInt();

    if ( umbral_b > 0 && umbral_a > 0 && umbral_a <= SPEED_MAX && umbral_b <= SPEED_MAX )
      PH_PID.SetOutputLimits(-umbral_a,+umbral_b);
    else
      PH_PID.SetOutputLimits(-SPEED_MAX, +SPEED_MAX);
  }
  //setting threshold temp: u2t130e
  else if ( message[1] == '2' ) {
    uint8_t umbral_temp = message.substring(3,6).toInt();

    if ( umbral_temp > 0 && umbral_temp <= SPEED_MAX )
      TEMP_PID.SetOutputLimits(0,+umbral_temp);
    else
      TEMP_PID.SetOutputLimits(0,SPEED_MAX);
  }

  Serial.println("umbral update good");
  return;
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
  Serial.print("\n");

  return;
}





void pid_temp() {
  Input_temp = Temp1;
  TEMP_PID.Compute();

  return;
}



void pid_ph() {
  Input_ph = pH;
  PH_PID.Compute();  //con esto se actualiza Output_ph

  //Algoritmo de seleccion de acido o base
  if ( Output_ph < 0 ) {
    ph_select = "a"; //=> Acido
    Output_ph = (int) -Output_ph;
  }
  else if ( Output_ph > 0 ) {
    ph_select = "b"; //=> Básico
    Output_ph = (int) +Output_ph;
  }



  return;
}



void setpoint() {
  //eventualmente, aca hay que programar el mezclador y usar crumble() para obtener el dato
  crumble();
  //acá se leen los nuevos setpoint para los pid
  Setpoint_temp = mytemp;
  Setpoint_ph   = myphset;

  Serial.println("good setpoint");

  return;
}




void broadcast_setpoint(uint8_t select) {
  //Re-transmition of NEW command to slave micro controller
  switch (select) {
    case 0: //only re-tx.
      mySerial.print(new_write);
      break;

    case 1: //update command and re-tx.
      format_message(Output_temp);
      t_temp = svar;

      format_message(Output_ph);
      t_ph = svar;

      new_write = "";
      new_write = message.substring(0,3) + ph_select + t_ph + message.substring(7,34) + t_temp + message.substring(37,55) + "\n";
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
  stringComplete = false; message = "";
  t_temp = "";
  t_ph = "";
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
          ( message.substring(11, 14).toInt() >= 0   ) &&
          ( message.substring(11, 14).toInt() <= SPEED_MAX ) &&

          //unload number
          ( message.substring(20, 23).toInt() >= 0   ) &&
          ( message.substring(20, 23).toInt() <= SPEED_MAX ) &&

          //mix number
          ( message.substring(26, 30).toInt() >= 0   ) &&
          ( message.substring(26, 30).toInt() <= 1500) &&

          //temp number
          ( message.substring(34, 37).toInt() >= 0   ) &&
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
      else if ( message[0] == 'u' && message[1] == '1' &&
                message[2] == 'a' && message[6] == 'b' &&
                message[10] == 'e'
              )
          return 1;

      //Validete umbral actuador temp: u2t003e
      else if ( message[0] == 'u' && message[1] == '2' &&
                message[2] == 't' && message[6] == 'e'
              )
          return 1;

      //Validate for pid parameters of temp and ph
      else if ( message[0] == 't' && (message[1] == '1' || message[1] == '2') &&
                message[2] == 'p' &&  message[9] == 'i' && message[16]== 'd'  &&
                message[23]== 'e'
              )
          return 1;

      // Validate READING
      else if ( message[0] == 'r' )
          return 1;

      // NOT VALIDATE
      else
          return 0;
}

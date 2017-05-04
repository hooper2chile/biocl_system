/*
  uc_slave
*/

#include "Arduino.h"
#include <avr/wdt.h>
#include <TimerOne.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); //RX(Digital2), TX(Digital3) Software serial port.


#define MIN_to_us 60e6 //60e6   [us]
#define STEPS     200 //200  [STEPS]
#define TIME_T    25  //TIME_T [us]

#define TIME_MIN  3000  //se despeja de SPEED_MAX
#define SPEED_MAX 100   //MIN_to_us/(STEPS*TIME_MIN)
#define SPEED_MIN 1

#define CONVERTION(x) ((unsigned int) (MIN_to_us/STEPS/TIME_T) / x) //x = speed [rpm] => Number of counts
#define LIMIT         (TIME_MIN / (2 * TIME_T) )

#define  INT(x) (x-48)  //ascii convertion
#define iINT(x) (x+48)  //inverse ascii convertion

//CHANGE IN FUNCTION THE UCONTROLLER, PORT AND PINS USED.
#define MOT1 PORTB
#define MOT2 PORTB
#define MOT3 PORTB
#define MOT4 PORTB
#define MOT5 PORTB

#define START1 PB0
#define START2 PB1
#define START3 PB2
#define START4 PB3
#define START5 PB4
//#define ALGO PB5 //reservado para mezclador (eventualmente)

#define PORT_CONTROL  PORTD
#define START_CONTROL PD6

//rst and dir port and pin setting
#define PORT_SETUP PORTC

//PH: las dos bombas de ph usan el mismo rst y dir
#define RST_PH     PC0
#define DIR_PH     PC1

//feed: comparte rst con unload
#define RST_FEED   PC2
#define DIR_FEED   PC3

#define RST_UNLOAD PC4

#define RST_TEMP   PC5
#define DIR_TEMP   PB5
//unload (no necesita DIR), unload se resetea junto con feed
//#define RST_MIX  PD7  //reservado para mezclador (eventualmente)

volatile uint16_t count_m1_set = 0;
volatile uint16_t count_m2_set = 0;
volatile uint16_t count_m3_set = 0;
volatile uint16_t count_m4_set = 0;
volatile uint16_t count_m5_set = 0;

uint16_t count_m1 = 0;
uint16_t count_m2 = 0;
uint16_t count_m3 = 0;
uint16_t count_m4 = 0;
uint16_t count_m5 = 0;

char    command = {};
String  message = "";
boolean stringComplete = false;

String  ph_var      = ""; String  ph_set      = "";
String  feed_var    = ""; String  feed_set    = "";
String  unload_var  = ""; String  unload_set  = "";
String  mix_var     = ""; String  mix_set     = "";
String  temp_var    = ""; String  temp_set    = "";
/*
* RESET, DIR SETUP
* rst3, dir2: myphset
* rst1, dir1: myfeed
* rst4, dir456: unload
* rst2, dir456: mymix
* rst5, dir3: mytemp
* rst, dir6: FREE !
*/
uint8_t rst1 = 0;  uint8_t rst2 = 0;  uint8_t rst3 = 0;
uint8_t rst4 = 0;  uint8_t rst5 = 0;  uint8_t rst6 = 0;

//DIRECTION SETUP
uint8_t dir1 = 1;  uint8_t dir2 = 1;  uint8_t dir3 = 1;
uint8_t dir4 = 1;  uint8_t dir5 = 1;  uint8_t dir6 = 1;

float   myphset  = 0;
uint8_t myfeed   = 0;
uint8_t myunload = 0;
uint8_t mymix    = 0;
uint8_t mytemp   = 0;

/*
//_BV(x) = 1 << x
inline void setup_dir_rst ( uint8_t *rst_var, uint8_t *dir_var, uint8_t *var, uint8_t RST, uint8_t DIR, volatile uint8_t *PORT ) {
  if( !(*rst_var) ) {
    if ( !(*var) )
      *PORT &= ~RST;
    else
      *PORT |=  RST;
  }
  else
    *PORT &= ~RST;

  if ( dir_var )
    *PORT |=  DIR;
  else
    *PORT &= ~DIR;
}
*/

//_BV(x) = 1 << x
inline void set_motor (uint16_t *count, uint16_t *count_set, volatile uint8_t *MOT, uint8_t START) {
  if ( *count == *count_set ) {
    *MOT |= START;
    *count = 0;
  }

  else if ( *count == LIMIT ) {
    *MOT &= ~START;
  }

  (*count)++;
  return;
}


inline void time_setup ( uint8_t motor_speed, volatile uint16_t *count_set, uint16_t *count ) {
  if ( !motor_speed ) {
    *count = 0;
    *count_set = 0;
    return;
  }

  else {
    *count = 0;
    //Transform of motor_speed [rpm] to time in [us]
    //SETTING FOR THRESHOLD OF TIME
    if      ( CONVERTION(motor_speed) <= CONVERTION(SPEED_MAX) )  *count_set = CONVERTION(SPEED_MAX);
    else if ( CONVERTION(motor_speed) >= CONVERTION(SPEED_MIN) )  *count_set = CONVERTION(SPEED_MIN);
    else                                                          *count_set = CONVERTION(motor_speed);

    return;
  }
}


//ISR: Function of Interruption in timer one.
void motor_control() {
  //PIN UP
  //PORT_CONTROL = 1 << START_CONTROL;

  // _BV(x) = 1 << x
  // set_motor(&count_m1, &count_m1_set, &MOT1, _BV(START1) );
  // set_motor(&count_m2, &count_m2_set, &MOT2, _BV(START2) );
  set_motor(&count_m3, &count_m3_set, &MOT3, _BV(START3) );
  set_motor(&count_m4, &count_m4_set, &MOT4, _BV(START4) );
  set_motor(&count_m5, &count_m5_set, &MOT5, _BV(START5) );

  //Pin DOWN
  //PORT_CONTROL = 0 << START_CONTROL;
}


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



void clean_strings() {
  //clean strings
  command    = {};   stringComplete = false;  message    = "";

  ph_var     = "";  feed_var       = "";  unload_var = "";  temp_var = "";
  mix_var    = "";  ph_set         = "";  feed_set   = "";  temp_set = "";
  unload_set = "";  mix_set        = "";

}


void crumble() {
  //Serial.println("good");
  command = message[0];

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



void setup() {
  wdt_disable();

  Serial.begin(9600);
  mySerial.begin(9600);

  //PORTs OUTPUT
  DDRB = 0b00111111;
  DDRC = 0b00111111;
  pinMode(10, OUTPUT); ///PD6 OUT: timer execution time

  Timer1.initialize(TIME_T);
  Timer1.attachInterrupt(motor_control);

  //reserve 32 bytes for the message:
  message.reserve(65);

  wdt_enable(WDTO_2S);
}


void loop() {

  if (stringComplete) {
    if (validate_write()) {
      Serial.println(message);
      //crumble the message
      crumble();

      //Time setup for counters
      //Lazo cerrado de Ph: rst3, dir2
      //time_setup(myfeed  , &count_m1_set, &count_m1);  //ph acido  pendiente, setear en otra función que reciba este mensaje desde un lazo de control
      //time_setup(myfeed  , &count_m2_set, &count_m2);  //ph basico pendiente, setear en otra función que reciba este mensaje desde un lazo de control

      /*
      * RESET, DIR SETUP
      * rst3, dir2: myphset
      * rst1, dir1: myfeed
      * rst4, dir456: unload
      * rst2, dir456: mymix
      * rst5, dir3: mytemp
      * rst6, dir6: FREE !
      */

      //Lazo abierto
      //feed: rst1=1 (enable); dir1=1 (cw), else ccw.
      if( !rst1 ) {
        if ( !myfeed )
          PORT_SETUP &= ~(1<<RST_FEED);
        else {
          PORT_SETUP |= 1<<RST_FEED;
          time_setup(myfeed, &count_m3_set, &count_m3);
        }
      }
      else {
        PORT_SETUP &= ~(1<<RST_FEED);
      }
      //direction feed
      if ( dir1 ) {
        PORT_SETUP |= 1<<DIR_FEED;
      }
      else PORT_SETUP &= ~(1<<DIR_FEED);



      //unload: rst4
      if( !rst4 ) {
        if ( !myunload )
          PORT_SETUP &= ~(1<<RST_UNLOAD);
        else {
          PORT_SETUP |= 1<<RST_UNLOAD;
          time_setup(myunload, &count_m4_set, &count_m4);
        }
      }
      else {
        PORT_SETUP &= ~(1<<RST_UNLOAD);
      }


      //temp: rst5, dir3
      if( !rst5 ) {
        if ( !mytemp )
          PORTC &= ~(1<<RST_TEMP);
        else {
          PORTC |= 1<<RST_TEMP;
          time_setup(mytemp, &count_m5_set, &count_m5);  //setear en otra función que reciba este mensaje desde un lazo de control
        }
      }
      else {
        PORTC &= ~(1<<RST_TEMP);
      }
      //direction temp
      if ( dir3 ) {
        PORTB |= 1<<DIR_TEMP;
      }
      else PORTB &= ~(1<<DIR_TEMP);



      //mix: rst2
      //time_setup(mymix,  &count_mX_set, &count_mX);  //hay que construir otra función para transmitir esta velocidad
      //Lazo cerrado de Temperatura: rst5, dir3

      //clean all strings
      clean_strings();

      wdt_reset();
    }

  }

}

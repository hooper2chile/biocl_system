/*
  * uc_slave
  *======================
  * RESET, DIR SETUP
  * rst3, dir2: myphset
  * rst1, dir1: myfeed
  * rst4, dir456: unload  (dir4 para unload, no se usa)
  * rst2, dir456: mymix
  * rst5, dir3: mytemp
  * rst6, dir6: FREE !
*/

#include <avr/wdt.h>
#include <TimerOne.h>
#include <slibrary.h>


void setup() {
  wdt_disable();

  Serial.begin(9600);

  DDRB = DDRB | (1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB4) | (1<<PB5);
  DDRC = DDRC | (1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3) | (1<<PC4) | (1<<PC5);
  DDRD = DDRD | (1<<PD4) | (1<<PD5) | (1<<PD6);

  Timer1.initialize(TIME_T);
  Timer1.attachInterrupt(motor_control);

  message.reserve(65);
  wdt_enable(WDTO_2S);
}



void loop() {
  if ( stringComplete )
  {
    if (validate_write()) {
      Serial.println(message);
      crumble();

      //Time setup for counters:
      //time_setup(ph1 , &count_m1_set, &count_m1);  //ph acido  pendiente, setear en otra función que reciba este mensaje desde un lazo de control
      //time_setup(ph2 , &count_m2_set, &count_m2);  //ph basico pendiente, setear en otra función que reciba este mensaje desde un lazo de control

      if ( myfeed != myfeed_save ) {
        time_setup(myfeed, &count_m3_set, &count_m3);
        myfeed_save = myfeed;
      }

      if ( myunload != myunload_save ) {
        time_setup(myunload, &count_m4_set, &count_m4);
        myunload_save = myunload;
      }

      if ( mytemp != mytemp_save ) {
        time_setup(mytemp, &count_m5_set, &count_m5);  //setear en otra función que reciba este mensaje desde un lazo de control
        mytemp_save = mytemp;
      }


      //RST and DIR SETTING:
      //Ph: rst3, dir2

      //feed: rst1=1 (enable); dir1=1 (cw), else ccw.
      setup_dir_rst( _BV(RST_FEED), _BV(DIR_FEED),
                     &myfeed, &rst1, &dir1,
                     &PORTC,  &PORTC );

      //unload: rst4, dir4=null, PORT_2 = NULL.
      setup_dir_rst( _BV(RST_UNLOAD), _BV(NULL),
                     &myunload, &rst4, NULL,
                     &PORTC,    &PORTC );

       //temp: rst5, dir3: PORT_1 distinto a PORT_2.
      setup_dir_rst( _BV(RST_TEMP), _BV(DIR_TEMP),
                     &mytemp, &rst5, &dir3,
                     &PORTC,  &PORTB );



      clean_strings();
      wdt_reset();
    }
  }


}

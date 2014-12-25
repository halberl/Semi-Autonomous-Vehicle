/*************************************************************
* Title: Project1 Semi Autonomous Vehicle
* Author:		Lucas Halbert
* Date:			12/01/14 Revision
* Description:	Create a semi-autonomous vehicle that can be
* 			  	trained to follow a designated path and play-
*				back the previously stored path.
*************************************************************/

#include <hidef.h>      /* common defines and macros */
#include <mc9s12dg256.h>     /* derivative information */
#pragma LINK_INFO DERIVATIVE "mc9s12dg256b"

#include "main_asm.h" /* interface to the assembly module */

#define MAXARRAYLENGTH 100
#define MOTORDDR DDRB
#define MOTORPORT PORTB

/**************  declarations  **************/
void display_movement(char **movement);         // displays a string to the LCD screen                        //
void playback_robot(void);                      // plays back the vehicles stored path
void get_keypad_input(void);                    // retrieves the user directional input from the keypad
int get_sensor_data(void);                      // retrieves sensor data
void add_to_array(int direction);               // while in "training mode" adds the direction to a storage array

/* Movement Functions */
void move_forward(void);                        // activates motors to move forward
void move_left(void);                           // activates motors to turn left
void move_right(void);                          // activates motors to turn right
void move_reverse(void);                        // activates motors to move backward
void move_stop(void);                           // deactivates motors to stop moving
void move_pause(void);                          // deactivates motors to pause moving

/*************** Global Variables **************/
int training=0;                                 // boolean value to enable/disable training mode
int playback=0;                                 // boolean value to enable/disable playback mode
int array_length=0;                             // counter to keep track of array location
int array1[MAXARRAYLENGTH]={9};                 // 1=forward, 2=left, 3=right, 4=reverse, 5=pause, 9=end of array


/* LCD screen variables */
char *forward="Forward";
char *stop="Stop";
char *left="Left";
char *right="Right";
char *reverse="Reverse";
char *pause="Pause";
char *error="THERE WAS AN ERROR";
char *start_playback="Playback started";
char *stop_playback="Playback stopped";
char *start_training="Training started";
char *stop_training="Training stopped";
char *array_empty="Array is Empty";
char *playback_mode="Robot is playing";



/**************  MAIN  **************/  
void main(void)
{
  seg7_disable();
  /* Stop Motors on initial load */
  MOTORDDR=0xFF;                    // enable motor port as output
  MOTORPORT=0xFF;                   // turn off all motor ports
  lcd_init();                       // initialize lcd
  set_lcd_addr(0x00);               // set starting lcd address
  
  /* check training or playback mode */
  while(1){
    if(SW2_down()){                 // switch2 starts training mode
      playback=0;
      training=1;
      display_movement(&start_training);
    }
    if(SW3_down()){                 // switch3 stops training mode
      training=0;
      display_movement(&stop_training);
    }
    if(SW4_down()){                 // switch4 starts playback mode
      if(array_length) {
        training=0;
        playback=1;
      } else {
        display_movement(&array_empty);
      }
    }
    if(SW5_down()){                 // switch5 erases array
      playback=0;
      array_length=0;               // reset array counter
        memset(array1, 9, sizeof(array1)); // reset array
      display_movement(&stop_playback);
    }
    get_keypad_input();
    playback_robot();
  }
}


/**************  prototypes  **************/

/*** Display movement ***/
void display_movement(char **movement) {
  clear_lcd();
  type_lcd(*movement);
}

/*** Add element to array ***/
void add_to_array(int direction){
  array1[array_length]=direction;
  array_length++;
}


/*** Playback Robot ***/
void playback_robot(){
  while(playback) {
    int i=0;
    display_movement(&start_playback);
    /* pull from array */
    for(i=0; i<=array_length; i++) {
      int direction=array1[i];
      
      if(direction == 1) {
        move_forward();
      }else if(direction == 2) {
        move_left();
      }else if(direction == 3) {
        move_right();
      }else if(direction == 4) {
        move_reverse();
      }else if(direction == 5) {
        move_pause();
      }else if((direction == 0) || (direction == 9)) {
        playback=0;
        display_movement(&stop_playback);
        break;
      } else {
        playback=0;
        break;
      }
      if(SW5_down()){         //switch5 stop playback mode
        playback=0;
        display_movement(&stop_playback);
      }
    }
  }
}


/*** get keypad input ****/
void get_keypad_input() {  
  char keypad_key;
  keypad_enable();
  while(training) {
    keypad_key=getkey();          // get value from keypad
    wait_keyup();
    lcd_init();
    set_lcd_addr(0x00);
    if(keypad_key==1) {           // 1 = forward
      move_forward();               
    }else if(keypad_key==4) {     // 4 = left
      move_left();
    }else if(keypad_key==7) {     // 7 = right
      move_right();
    }else if(keypad_key==0) {     // 0 = reverse
      move_reverse();
    }else if(keypad_key==6) {     // 7 = pause
      move_pause();
    }else if(keypad_key==9) {     // break out of training mode
      training=0;
      move_stop();
      display_movement(&stop_training);
      break;
    }else{
      display_movement(&error);
    }
    ms_delay(50);
  }
}


/*** Move forward ***/
void move_forward() {
  int i=0;
  int sensor_detect=0;
  for(i=0; i!=8; i++) {           // loop 8 times and detect objects every 250 ms
    display_movement(&forward);
    
    /* Detect any objects in the way */
    sensor_detect = get_sensor_data();
    while(sensor_detect){
      MOTORPORT=0xFF;
      //stop execution of queue
      ms_delay(250);
      sensor_detect = get_sensor_data();
    }
    
    MOTORDDR=0xFF;                // enable motor port as output
    MOTORPORT=0xFA;               // turn on motor port(0)
    ms_delay(250);
  }
  if(training) {                  // if training mode boolean is 1, add direction to array
    add_to_array(1);
  }
  move_stop();
}

/*** Move left ***/
void move_left() {
  int i=0;
  for(i=0; i!=1; i++) {
    display_movement(&left);
    MOTORDDR=0xFF;                // enable motor port as output
    MOTORPORT=0xFE;               // turn on motor port(2)
    ms_delay(850);
  }
  if(training) {                  // if training mode boolean is 1, add direction to array
    add_to_array(2);
  }
  move_stop();
}

/*** Move right ***/
void move_right() {
  int i=0;
  for(i=0; i!=1; i++) {
    display_movement(&right);
    MOTORDDR=0xFF;                // enable motor port as output
    MOTORPORT=0xFB;               // turn on motor port(1)
    ms_delay(850);
  }
  if(training) {                  // if training mode boolean is 1, add direction to array
    add_to_array(3);
  }
  move_stop();
}

/*** Move reverse ***/
void move_reverse() {
  display_movement(&reverse);
  MOTORDDR=0xFF;                  // enable motor port as output
  MOTORPORT=0xF5;                 // turn off all motor ports
  ms_delay(2000);
  if(training) {
    add_to_array(4);              // if training mode boolean is 1, add direction to array
  }
  move_stop(); 
}


/*** Move stop ***/
void move_stop() {
  display_movement(&stop);
  MOTORDDR=0xFF;                  // enable motor port as output
  MOTORPORT=0xFF;                 // turn off all motor ports
}

/*** Move pause ***/
void move_pause() {
  display_movement(&pause);
  MOTORDDR=0xFF;                  // enable motor port as output
  MOTORPORT=0xFF;
  ms_delay(2000);
  if(training) {
    add_to_array(5);              // if training mode boolean is 1, add direction to array
  }
  move_stop();                    // turn off all motor ports
}



/*** Get Sensor Data ***/
int get_sensor_data(void) {
  int sensor_detect=1;            // initialize sensor detect to 1 (nothing detected)
  DDRP = 0x00;                    // enable port P as input
  if(PTP == 0x00){               
    sensor_detect = 1;            // if sensor returns a 0, object detected
  } else {
    sensor_detect = 0;            // if sensor returns a 1, no object detected
  }
  return sensor_detect;
}



		
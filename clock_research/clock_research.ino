#include <FlexiTimer2.h>

int counter;

boolean flasher1 = HIGH; 
boolean flasher2 = HIGH; 

#define PPQ 24

// when the sequencer starts, set the timer with the new tempo, something like 75 or 125, a normal bpm.

void setTimer(int beatsPerMinute){
  
  // sync period in milliSeconds
 int period = ((1000L * 60)/beatsPerMinute)/(PPQ);

 
  FlexiTimer2::set(period, syncMIDI);
  FlexiTimer2::start();                                                                                               
}
void setup(){setTimer(120);
Serial.begin(28800); // DEBUG 
  Serial1.begin(31250); // midi
 Serial1.write(0xFA); 
 pinMode(46, OUTPUT);
 
 pinMode(44, OUTPUT);
}
void loop(){


}

// Called by timer
void syncMIDI()
{
   Serial1.write(0xF8); // Midi Clock Sync Tick  
   flash1();
if (counter>23){
flash2();
counter = 0;}
counter++;

}

void flash1(){
 digitalWrite(46, flasher1);
 flasher1 = !flasher1; 
}

void flash2(){
 digitalWrite(44, flasher2);
 flasher2 = !flasher2; 
 
int update_clock = map(analogRead(0),0,1024,60,550);
setTimer(update_clock);
 
}

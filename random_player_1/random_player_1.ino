#include <FlexiTimer2.h>



/*
Random piano Sequencer 
  
 Knobs: 
 0 = TEMPO
 1 = number of notes changed each 128 note cycle
 2 = mode selection 
 5 & 6 = min and max velocity
 3 = loop length 
 
Schematic in pot_box_schematic 
 
To do: 
Create collection of note change functions; 
	change note randomly (range)
	change velocity randomly (range) 
	change duration randomly (range) 
	Change note/velocity/duration based on current value (is this a special case of markov below?)
	remove note 
	add new note (random note/velocity/duration) 
	add new note (note/velocity/duration related to previous note(s) - markov)
	Add new note (note/velocity/duration related to rhythm system)
	Change all notes above/below a specific velocity
Create a system that allows new events to happen every n repetitions. 
	i.e. not on every loop
Create standard internal numbering system (i.e. 0-256) that can be applied to any variable.
	ie create a LFO which can be applied to note/velocity/duration 
	build markov chain around this variable
		so you can repeat/learn velocity chains as easily as note chains 
	BUT: How to connect 0-256 number with 0-7+0-8 note+octave system? 
System to change a particular global variable for a period 
	i.e. change base note / mode / TEMPO for n bars, then revert 
	Is it a temporary change vs a permanent change - maybe with a buffer to reverse the change? 
Rework rhythm system - enable CHANGES of rhythm 
	(Could be covered by note change function above, i.e. ChangeVelocityByRhythm)
Rework timing system. 
        Use 24ppq system, to enable midi clock output? 
Rework Quantise system to ensure it correctly handles differently-sized scales
Rework Rhythm system to use smaller arrays 
Create chord system, scanning current notes playing and running chords over the top
X freeze() and unFreeze() = copies current loop into a buffer. Unfreeze copies it back into the live loop. 

Experiments to try: 
        Create change systems for note/velocity/duration and attach them to knobs 

 */

//*************
//GLOBAL VARIABLES 
//*************

#define ARRAY_SIZE 128
// ARRAY_SIZE must match the ODDS array and is the maximum sequence length 

byte MODE_CHOICE;
byte ODDS_CHOICE;
byte noteplay;
int TEMPO  = 120;
int MAX_TEMPO=300;
int MIN_TEMPO = 8;
int PERIOD; 
int DENSITY =100; // 100 = average, 200 = low, 50 = high
int MAX_DENSITY = 200;
int MIN_DENSITY = 10;
byte MIN_VELOCITY;
byte MAX_VELOCITY;
int LOOP_LENGTH = ARRAY_SIZE;
int CHANGES = 0;
boolean FILL=true;
boolean DEBUG = false; 
boolean PRINT_SETTINGS = true; 
#define greenLED2 44
#define greenLED1 45
#define redLED 46
int REPEAT_COUNT;
byte BASE_TIME; // multiplied by tempo for note lengths 
int CONTROL_REFRESH = 2000;
#define PPQ 24 // master PPQ of the sequencer 
byte PLAY_DIVIDER = 3; //play a note every n parts 
byte PLAY_COUNTER;
long loop_count = 0; 

long OLD;
long NEW;



// holder for the sequences  
#define SEQUENCE_LAYERS 3 //0=note 1=octave 2=velocity
#define VOICE_COUNT 16 // maximum polyphony before note off system breaks down (to limit memory use)  
byte SEQUENCE[SEQUENCE_LAYERS][ARRAY_SIZE];   
byte FROZEN_SEQUENCE[SEQUENCE_LAYERS][ARRAY_SIZE];  
byte NOTE_ON[VOICE_COUNT];
unsigned long NOTE_OFF[VOICE_COUNT];
int SEQUENCE_STEP;

//*************
//LOOKUP TABLES 
//*************


//QUANTISATION
#define MODE_COUNT 18 
byte MODES[MODE_COUNT][12]= {
  {0,1,2,3,4,5,6,7,8,9,10,11},// chromatic 
  {0,2,4,5,7,9,11,12},        // major 
  {0,2,4,5,8,9,11,12},        // harmonic major 
  {0,2,4,7,9,12,14},          // pentatonic major 
  {0,2,3,5,7,8,10,12},        // minor 
  {0,2,3,5,7,8,11,12},        // harmonic minor
  {0,3,5,7,10,12,15},         // pentatonic minor  
  {0,2,3,5,6,8,9,11},         // diminished
  {0,2,4,6,8,10,12},          // whole tone 
  {0,3,5,6,7,10,12},          // blues 
  {0,1,3,5,7,9,10,12},        // javanese
  {0,2,4,5,6,8,10,12},        // arabian 
  {0,1,4,5,6,8,11,12},        // persian 
  {0,1,4,5,6,9,10,12},        // oriental 
  {0,1,4,5,7,8,10,12},        // jewish 
  {0,1,4,5,7,8,11,12},        // gypsy
  {0,2,3,5,7,9,10,12},        // dorian 
  {0,2,4,5,7,8,11,12},        // ethiopian 
};  


#define ODDS_COUNT 6
byte ODDS[ODDS_COUNT][ARRAY_SIZE]={
/* 
0 = 4/4
1 = clock divisions 
2 = samba
3 = 3/4
4 = cuban
5 = bossanova 
*/
{95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5},
{95,5,95,5,95,5,95,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,5,5,5,5,95,5,5,5,5,5,5,5,95,5,5,5,5,5,5,5,95,5,5,5,5,5,5,5,95,95,95,95,95,95,95,95,95,5,95,5,95,5,95,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,5,5,5,5,95,5,5,5,5,5,5,5,95,5,5,5,5,5,5,5,95,5,5,5,5,5,5,5,95,95,95,95,95,95,95,95,},
{95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,},
{90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20},
{95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,},
{95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,}};



//*************
//  SETUP  
//*************

void setup() {
    Serial.begin(115200); // DEBUG 
    Serial1.begin(31250); // midi 
    randomSeed(analogRead(9));
    pinMode(redLED,OUTPUT);
    pinMode(greenLED1,OUTPUT);
    pinMode(greenLED2,OUTPUT);
    setTimer(TEMPO);
  FlexiTimer2::start(); 


}


//*************
// MAIN LOOP  
//*************

void loop() {


if(FILL == true){
randomiseValues();
fillRandom();
FILL = false;
}
  
if (loop_count%CONTROL_REFRESH == 0){  
readKnobs();
}



// EVERY 96 NOTES, FREEZE AND REFILL  
  if(REPEAT_COUNT==96){  
   freeze();
    fillRandom();}
    if(REPEAT_COUNT == 192){
      unFreeze();
      REPEAT_COUNT = 0;};


// MAKE CHANGES EVERY 128 NOTES **NEED TO FIX THIS, WILL RUN EVERY NOTE, CAN SIMPLIFY TO CHANGE ONE NOTE EACH CYCLE 
//
//for (int i = 0; i<CHANGES; i++){
//byte changer=random(ARRAY_SIZE); // choose an entry to change 
//if(random(DENSITY)<ODDS[ODDS_CHOICE][changer] && SEQUENCE[2][changer]>0){ // if a rhythmically interesting note and non-silent 
//SEQUENCE[0][changer] = SEQUENCE[0][changer]+(random(3)-1);}}




loop_count++;


}


//*************
//FUNCTIONS 
//*************

// midi note player 

void playNote(byte cmd, byte pitch, byte velocity, int duration) {
unsigned long time = millis();
boolean found_slot = false;
for (int i=0;i<VOICE_COUNT;i++){
if (NOTE_ON[i] == 0){  
  NOTE_ON[i] = pitch;
  NOTE_OFF[i] = time+duration;
  found_slot = true;
  break;
}}

if (found_slot == false){digitalWrite (redLED, HIGH);} // RED LIGHT SHOWS NOTE BUFFER IS OVERFULL 
else{digitalWrite (redLED, LOW);};
  
  Serial1.write(cmd);
  Serial1.write(pitch);
  Serial1.write(velocity);
  if (DEBUG == true && velocity>0){  
  Serial.print("Pitch ");
  Serial.print(pitch);
  Serial.print(" Velocity ");
  Serial.println(velocity);
  }
}

// Check for held Notes that should be killed, and kill them. 

void noteKill(){
 unsigned long time=millis();
 for (int i=0;i<VOICE_COUNT;i++){
 if (NOTE_ON[i]>0 && NOTE_OFF[i]<time){  // if note value>0 and note off time < time
 byte pitch = NOTE_ON[i];
 byte velocity = 0;
 Serial1.write(0x90);
 Serial1.write(pitch);
 Serial1.write(velocity);
 NOTE_ON[i]=0; //wipe NOTE_ON entry to signal no active note
}}}


// Quantizer 

byte quantize(int scale, int note, int octave){
  return octave*12+MODES[scale][note]; 
}

// FREEZE / UNFREEZE 

void freeze(){
 for(int i=0;i<ARRAY_SIZE;i++){
 for(int x=0;x<SEQUENCE_LAYERS;x++){ 
   FROZEN_SEQUENCE[x][i] =SEQUENCE [x][i];
 }} 
digitalWrite(greenLED2, HIGH); 
}


void unFreeze(){
 for(int i=0;i<ARRAY_SIZE;i++){
 for(int x=0;x<SEQUENCE_LAYERS;x++){ 
   SEQUENCE [x][i]=FROZEN_SEQUENCE[x][i] ;
 }}   
digitalWrite(greenLED2, LOW); 
}

// FILL ENTIRE LOOP WITH RANDOM NOTES 

void fillRandom(){
for(int SEQUENCE_STEP=0;SEQUENCE_STEP<ARRAY_SIZE;SEQUENCE_STEP++){
    byte newnote=random(7);
    byte newoctave = random(5)+3;
    if(random(DENSITY)<ODDS[ODDS_CHOICE][SEQUENCE_STEP]){
    SEQUENCE[0][SEQUENCE_STEP]= newnote; // note 
    SEQUENCE[1][SEQUENCE_STEP]= newoctave;  // octave 
    SEQUENCE[2][SEQUENCE_STEP]= random(ODDS[ODDS_CHOICE][SEQUENCE_STEP]+27); // velocity 
}}}

//  RETURN VELOCITY WITHIN LIMITS SET BY POTS 
int getVelocity(int velocity){
if (velocity>0){
if (velocity<=MIN_VELOCITY){velocity=MIN_VELOCITY;};
if (velocity>=MAX_VELOCITY){velocity=MAX_VELOCITY;};
}
return velocity;
}


// RANDOMISE MASTER VARIABLES 
void randomiseValues(){
MODE_CHOICE=random(MODE_COUNT);
ODDS_CHOICE = random(ODDS_COUNT);
DENSITY = (random(MAX_DENSITY-MIN_DENSITY))+MIN_DENSITY; 
BASE_TIME = random(16); 
}

// READ KNOBS 
void readKnobs(){
 TEMPO = map(analogRead(0),0,1024,MIN_TEMPO,MAX_TEMPO);
CHANGES = map(analogRead(1),0,1024,0,ARRAY_SIZE); 
MODE_CHOICE = map(analogRead(2),0,1024,0,MODE_COUNT);
LOOP_LENGTH = (analogRead(3)/128)*8;
MIN_VELOCITY = map(analogRead(4),0,1024,1,127);
MAX_VELOCITY = map(analogRead(5),0,1024,MIN_VELOCITY,127);

}


void setTimer(int beatsPerMinute){
  
  // sync period in milliSeconds
  PERIOD = ((1000L * 60)/beatsPerMinute)/(PPQ);
  FlexiTimer2::set(PERIOD, playSequenceNote);
  
}


void playSequenceNote(){
if (PLAY_COUNTER == PLAY_DIVIDER){

byte velocity = getVelocity(SEQUENCE[2][SEQUENCE_STEP]);
int note_length = PERIOD*PLAY_DIVIDER*random(10);
byte this_note = quantize(MODE_CHOICE, SEQUENCE[0][SEQUENCE_STEP], SEQUENCE[1][SEQUENCE_STEP]);
playNote(0x90, this_note,velocity,note_length);
noteKill();
if (SEQUENCE_STEP%4==0){digitalWrite(greenLED1, HIGH);}else{digitalWrite(greenLED1, LOW);}
REPEAT_COUNT++;
SEQUENCE_STEP++;
if (SEQUENCE_STEP>LOOP_LENGTH){
 SEQUENCE_STEP = 0; 
}

setTimer(TEMPO);
  FlexiTimer2::start(); 
PLAY_COUNTER = 0;
}
PLAY_COUNTER++;
}



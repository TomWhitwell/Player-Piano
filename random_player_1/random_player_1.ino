#include <FlexiTimer2.h>



/*
Random piano Sequencer 
  
 Knobs: 
 0 = TEMPO
 1 = unused 
 2 = unused  
 5 & 6 = min and max velocity
 3 = loop length 
 
Schematic in pot_box_schematic.
 
To do: 
Create collection of note change functions; 
	X change note randomly (range)
	change velocity randomly (range) 
	change duration randomly (range) 
	Change note/velocity/duration based on current value (is this a special case of markov below?)
	remove note 
	X add new note (random note/velocity/duration) 
	add new note (note/velocity/duration related to previous note(s) - markov)
	Add new note (note/velocity/duration related to rhythm system)
	X Change all notes above/below a specific velocity
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
X Rework timing system. 
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
int MAX_TEMPO=400;
int MIN_TEMPO = 8;
int PERIOD; 
int DENSITY =100; // 100 = average, 200 = low, 50 = high
int MAX_DENSITY = 200;
int MIN_DENSITY = 10;
byte MIN_VELOCITY;
byte MAX_VELOCITY;
int LOOP_LENGTH = ARRAY_SIZE;
int CHANGES = 0;
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
boolean FROZEN = false;
long loop_count = 0; 
byte LOOP_STABILITY = 2; // How likely individual notes are to change as they play 

long OLD;
long NEW;
byte NOTE_ACTION_1;
byte NOTE_ACTION_2;
byte LOOP_ACTION_1;
byte LOOP_ACTION_2;
byte LOOP_EVERYNOTE_ACTION_1;
byte LOOP_EVERYNOTE_ACTION_2;


// FUNCTION DEFAULTS 
void changeRandomNoteAt(int note_position, byte low_velocity = 0, byte high_velocity = 127);


// FLOW CONTROL 
boolean START = true; 
boolean FILL = false;
boolean NOTE = false;
boolean LOOP = false; 
boolean SECTION = false; 
int SECTION_LENGTH;
int LOOPS_IN_SECTION = 16;



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
  {0,2,4,6,8,10,12},          // whole tone 
  {0,2,4,5,7,9,11,12},        // major 
  {0,2,4,5,8,9,11,12},        // harmonic major 
  {0,2,4,7,9,12,14},          // pentatonic major 
  {0,2,3,5,7,8,10,12},        // minor 
  {0,2,3,5,7,8,11,12},        // harmonic minor
  {0,3,5,7,10,12,15},         // pentatonic minor  
  {0,2,3,5,6,8,9,11},         // diminished
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

char MODE_NAMES[MODE_COUNT][12]=
{
 "Whole Tone",
"Major",
"HarmMajor",
"PentMajor",
"Minor",
"HarmMinr",
"PentMinor",
"Diminished",
"Blues",
"Java",
"Arabian",
"Persian",
"Oriental",
"Jewish",
"Gypsy",
"Dorian",
"Ethiopian"};

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
char ODDS_NAMES[ODDS_COUNT][15]=
  {"4/4",
  "clock divns",
  "samba",
  "3/4",
  "cuban",
  "Bossanova"};


#define DIVIDER_COUNT 9
int DIVIDERS[DIVIDER_COUNT] = 
{1,2,4,8,16,32,48,64,128};


//*************
//  SETUP  
//*************

void setup() {
    Serial.begin(115200); // DEBUG 
    Serial.println("**************");
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

if (loop_count%CONTROL_REFRESH == 0){  
readKnobs();
boolean any_notes = false;
for (int i=0; i<LOOP_LENGTH; i++){
  if (SEQUENCE[2][i] > 0){
   any_notes = true; 
  }
}
if (any_notes == false){
  addRandomNoteAt(random(LOOP_LENGTH));
}
}


if(START == true){
    Serial.println("START");
randomiseValues();
defineActions();
addRandomNoteAt(random(LOOP_LENGTH));
START = false;
}


if(FILL == true){
    Serial.println("FILL");

fillRandom();
FILL = false;
}

if (NOTE == true){

  selectNoteAction(NOTE_ACTION_1, SEQUENCE_STEP);
  selectNoteAction(NOTE_ACTION_2, SEQUENCE_STEP);
  

NOTE = false;  
}


if (LOOP == true){Serial.println("LOOP");

  selectSectionAction(LOOP_ACTION_1, random(LOOP_LENGTH));
//  selectSectionAction(LOOP_ACTION_2, random(LOOP_LENGTH));



 
LOOP = false;  
LOOPS_IN_SECTION++;
if (LOOPS_IN_SECTION>SECTION_LENGTH-1){
LOOPS_IN_SECTION = 0;
SECTION = true;
}}


if (SECTION == true){ Serial.println("SECTION");
 
 for (int x = 0; x < LOOP_LENGTH; x++){

  selectNoteAction(LOOP_EVERYNOTE_ACTION_1, x);
    selectNoteAction(LOOP_EVERYNOTE_ACTION_2, x);
}
 
defineActions();

if (random(10)>8){
randomLoopLength();
Serial.print(" Loop length = ");
Serial.println(LOOP_LENGTH);
};
  
 SECTION = false;  
}


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
if (NOTE_ON[i] == 0 || NOTE_ON[i] == pitch){  
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
if (FROZEN == false){
 
  for(int i=0;i<ARRAY_SIZE;i++){
 for(int x=0;x<SEQUENCE_LAYERS;x++){ 
   FROZEN_SEQUENCE[x][i] =SEQUENCE [x][i];
 }} 
digitalWrite(greenLED2, HIGH); 
FROZEN = true;
}
else{
 for(int i=0;i<ARRAY_SIZE;i++){
 for(int x=0;x<SEQUENCE_LAYERS;x++){ 
   SEQUENCE [x][i]=FROZEN_SEQUENCE[x][i] ;
 }}   
digitalWrite(greenLED2, LOW); 
FROZEN = false;
}}

// FILL ENTIRE LOOP WITH RANDOM NOTES 

void fillRandom(){
for(int fill_step=0;fill_step<ARRAY_SIZE;fill_step++){
    if(random(DENSITY)<ODDS[ODDS_CHOICE][fill_step]){
    byte newnote=random(7);
    byte newoctave = random(5)+3;
    SEQUENCE[0][fill_step]= newnote; // note 
    SEQUENCE[1][fill_step]= newoctave;  // octave 
    SEQUENCE[2][fill_step]= random(ODDS[ODDS_CHOICE][fill_step]+27); // velocity 
}}}


/*
Document note creation/modification functions: 

addRandomNoteAt(note_position)
= Checks to see if there is a note present. If not, assesses the rhythm system, throws a dice, and if successful, adds a new random note in the relevant position. 

changeRandomNoteAt(note_position,low_velocity,high_velocity)
= Checks to see if there is a note present. 
Optional: Checks whether note is above low_velocity. 
Optional: Checks whether note is above low and below high_velocity. 
If true, assesses the rhythm system, throws a dice, and 
If dice throw is successful, changes to a new random note in the relevant position.
If dice throw is unsuccessful, leave as was. 


incrementNoteAt(note_positon)
= raises note by one, raising octave after 7 notes, looping through octaves 

decrementNoteAt(note_position)
= lowers note by one, reducing octave after 7 notes, looping through octaves 

simplifyNoteAt(note_position)
= Creates a note if there is not one, or changes an existing note. 
If the rhythm step is >50, it creates a root note at octave 5

repeatNoteBefore(note_position)
= checks back for the last active note before the specified one. Does not loop around. 
Repeats exactly and does not check rhythm before adding a note. 

addRepeatedNote(note_position)
= checks back for the last active note before the specified one. Does not loop around. 
Checks rhythm and adds new note with same note/octave as the last, but new velocity. 

randomLoopLength()
= randomises the loop length 

clearBelowVelocity(note_position, 


*/

void addRandomNoteAt(int note_position){
  if (note_position<ARRAY_SIZE && SEQUENCE[2][note_position] == 0){
     if(random(DENSITY)<ODDS[ODDS_CHOICE][note_position]){
    byte newnote=random(7);
    byte newoctave = random(5)+3;
    SEQUENCE[0][note_position]= newnote; // note 
    SEQUENCE[1][note_position]= newoctave;  // octave 
    SEQUENCE[2][note_position]= random(ODDS[ODDS_CHOICE][note_position]+27); // velocity 
}}}

void changeRandomNoteAt(int note_position, byte low_velocity, byte high_velocity){
   if (note_position<ARRAY_SIZE 
   && SEQUENCE[2][note_position] > low_velocity
   && SEQUENCE[2][note_position] <= high_velocity){
     if(random(DENSITY)<ODDS[ODDS_CHOICE][note_position]){
    byte newnote=random(7);
    byte newoctave = random(5)+3;
    SEQUENCE[0][note_position]= newnote; // note 
    SEQUENCE[1][note_position]= newoctave;  // octave 
    SEQUENCE[2][note_position]= random(ODDS[ODDS_CHOICE][note_position]+27); // velocity 
}}}


void incrementNoteAt(int note_position){
    if (note_position<ARRAY_SIZE && SEQUENCE[2][note_position] > 0){
    SEQUENCE[0][note_position]++;  
if (SEQUENCE[0][note_position]>7){
  SEQUENCE[0][note_position] = 0;
  SEQUENCE[1][note_position]++;}
  if (SEQUENCE[1][note_position] > 8){
    SEQUENCE[1][note_position] = 3;
}}}

void decrementNoteAt(int note_position){
    if (note_position<ARRAY_SIZE && SEQUENCE[2][note_position] > 0){
if (SEQUENCE[0][note_position]==1){
  SEQUENCE[0][note_position] = 7;
  SEQUENCE[1][note_position]--;}
 else {SEQUENCE[0][note_position]--;};
  if (SEQUENCE[1][note_position] < 3){
    SEQUENCE[1][note_position] = 8;
}}}

void simplifyNoteAt(int note_position){
if(ODDS[ODDS_CHOICE][note_position]>50){
 SEQUENCE[0][note_position] = 0;
 SEQUENCE[1][note_position] = 5;
 SEQUENCE[2][note_position] = 50;}
 else{
 SEQUENCE[0][note_position] = 0;
 SEQUENCE[1][note_position] = 4;
 SEQUENCE[2][note_position] = 40;}
   
 }

  void repeatNoteBefore(int note_position){
    if (note_position > 1){
   for (int i=note_position-1; i>=0;i--){
    if(SEQUENCE[2][i]>0){
     for (int j = 0;j<SEQUENCE_LAYERS;j++){
      SEQUENCE[j][note_position] = SEQUENCE[j][i];
     }
      break;
  }}}}
  
  void addRepeatedNote(int note_position){
    if (note_position > 1){
   for (int i=note_position-1; i>=0;i--){
    if(SEQUENCE[2][i]>0){
         if(random(DENSITY)<ODDS[ODDS_CHOICE][note_position]){
    SEQUENCE[0][note_position]= SEQUENCE[0][i]; // note 
    SEQUENCE[1][note_position]= SEQUENCE[1][i];  // octave 
    SEQUENCE[2][note_position]= random(ODDS[ODDS_CHOICE][note_position]+27); // velocity 
         }
    }
      break;
  }}}
  
  void randomLoopLength(){
   LOOP_LENGTH = DIVIDERS[random(DIVIDER_COUNT)]; 
  }
  
  
  
  
#define SECTION_ACTIONS_COUNT 9
void selectSectionAction(byte choice, byte note_position){
 switch (choice){
   case 0:
   addRandomNoteAt(note_position);
   Serial.print(" add ");
   break;
   case 1:
   changeRandomNoteAt(note_position);
   Serial.print(" change ");
   break;
   case 2:
   incrementNoteAt(note_position);
      Serial.print(" up ");
   break;
   case 3:
     decrementNoteAt(note_position); 
           Serial.print(" down ");
   break;
   case 4:
   addRepeatedNote(note_position);
         Serial.print(" repeat ");
   break;
   case 5:
//   simplifyNoteAt(note_position);
//         Serial.print(" simplify ");
   break;
   case 6:
ODDS_CHOICE = random(ODDS_COUNT);
            Serial.print(" re-rhythm ");
   break;
   case 7:
//fillRandom();
//Serial.print (" fill ");
   break;
   case 8:
   freeze();
Serial.print (" freeze ");
   break;
   case 9:
   break;
   }}


#define NOTE_ACTIONS_COUNT 7
void selectNoteAction(byte choice, byte note_position){
 if(choice <= NOTE_ACTIONS_COUNT){
  
  switch (choice){
   case 0:
   addRandomNoteAt(note_position);
   Serial.print(" Nadd ");
   break;
   case 1:
   changeRandomNoteAt(note_position);
   Serial.print(" Nchange ");
   break;
   case 2:
   incrementNoteAt(note_position);
      Serial.print(" Nup ");
   break;
   case 3:
     decrementNoteAt(note_position); 
           Serial.print(" Ndown ");
   break;
   case 4:
   addRepeatedNote(note_position);
         Serial.print(" Nrepeat ");
   break;
   case 5:
//   simplifyNoteAt(note_position);
//         Serial.print(" Nsimplify ");
   break;
   case 6:
      changeRandomNoteAt(note_position,60);
            Serial.print(" NchngHI ");
   break;
   case 7:
      changeRandomNoteAt(note_position,0,60);
Serial.print (" NchngLO ");
   break;
  }}
else {}
}




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
BASE_TIME = random(8); 
if (random(4)>1){FILL = true;}else{FILL = false;};
SECTION_LENGTH = DIVIDERS[random(DIVIDER_COUNT)];
LOOP_LENGTH = DIVIDERS[random(DIVIDER_COUNT)];
LOOP_STABILITY = random(10);

if (PRINT_SETTINGS == true){
Serial.print(" Density = ");
Serial.print(DENSITY);
Serial.print(" fill = ");
Serial.print(FILL);
Serial.print(" Base Times = ");
Serial.print(BASE_TIME);}
Serial.print(" Rhythm = ");
Serial.print(ODDS_NAMES[ODDS_CHOICE]);
Serial.print(" Section length = ");
Serial.print(SECTION_LENGTH);
Serial.print(" Mode = ");
Serial.print(MODE_NAMES[MODE_CHOICE]);
Serial.print(" Loop stability = ");
Serial.print(LOOP_STABILITY);
Serial.print(" Loop length = ");
Serial.println(LOOP_LENGTH);

}

// DEFINE ACTIONS 
void defineActions(){
LOOP_ACTION_1 = random(SECTION_ACTIONS_COUNT);
LOOP_ACTION_2 = random(SECTION_ACTIONS_COUNT);
NOTE_ACTION_1 = random(NOTE_ACTIONS_COUNT*LOOP_STABILITY); 
NOTE_ACTION_2 = random(NOTE_ACTIONS_COUNT*LOOP_STABILITY);
Serial.println("");
Serial.print ("note action 1 = ");
Serial.println (NOTE_ACTION_1);
Serial.print ("note action 2 = ");
Serial.println (NOTE_ACTION_2);

LOOP_EVERYNOTE_ACTION_1 = random(NOTE_ACTIONS_COUNT);
LOOP_EVERYNOTE_ACTION_2 = random(NOTE_ACTIONS_COUNT);
}

// READ KNOBS 
void readKnobs(){
 TEMPO = map(analogRead(0),0,1024,MIN_TEMPO,MAX_TEMPO);
//CHANGES = map(analogRead(1),0,1024,0,ARRAY_SIZE); 
//MODE_CHOICE = map(analogRead(2),0,1024,0,MODE_COUNT);
//LOOP_LENGTH = (analogRead(3)/128)*8;
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

int note_length = PERIOD*PLAY_DIVIDER*BASE_TIME;
byte this_note = quantize(MODE_CHOICE, SEQUENCE[0][SEQUENCE_STEP], SEQUENCE[1][SEQUENCE_STEP]);
playNote(0x90, this_note,velocity,note_length);
noteKill();
if (SEQUENCE_STEP%4==0){digitalWrite(greenLED1, HIGH);}else{digitalWrite(greenLED1, LOW);}
REPEAT_COUNT++;
SEQUENCE_STEP++;
NOTE=true;
if (SEQUENCE_STEP>LOOP_LENGTH-1){
 SEQUENCE_STEP = 0; 
 LOOP = true; 
}

setTimer(TEMPO);
  FlexiTimer2::start(); 
PLAY_COUNTER = 0;
}
PLAY_COUNTER++;
}



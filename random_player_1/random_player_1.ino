/*
Random piano Sequencer 
 
 Notes on this version: 
 - Sequencer looping
 
 Knobs: 
 0 = TEMPO
 1 = number of MODES changed each 128 note cycle
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
	Change all MODES above/below a specific velocity
Create a system that allows new events to happen every n repetitions. 
	i.e. not on every loop
Create standard internal numbering system (i.e. 0-256) that can be applied to any variable.
	ie create a LFO which can be applied to note/velocity/duration 
	build markov chain around this variable
		so you can repeat/learn velocity chains as easily as note chains 
	BUT: How to connect 0-256 number with 0-7+0-8 note+octave system? 
System to change a particular global variable for a period 
	i.e. change base note / mode / TEMPO for n bars, then revert 
	Is it a TEMPOrary change vs a permanent change - maybe with a buffer to reverse the change? 
Rework rhythm system - enable changes of rhythm 
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



/*
GLOBAL VARIABLES 
*/
byte MODE_CHOICE;
#define MODE_COUNT 18

byte ODDS_CHOICE;
#define ODDS_COUNT 6

byte noteplay;
#define ARRAY_SIZE 128

int TEMPO  = 60;
int MAX_TEMPO=0;
int MIN_TEMPO = 200;

int DENSITY =100; // 100 = average, 200 = low, 50 = high
int MAX_DENSITY = 200;
int MIN_DENSITY = 10;

boolean FILL=true;
boolean DEBUG = false; 
boolean PRINT_SETTINGS = true; 

#define greenLED2 44
#define greenLED1 45
#define redLED 46

#define VOICE_COUNT 16 // maximum polyphony 

int REPEAT_COUNT;


byte LONG_SHORT;
byte BASE_TIME; // multiplier for note lengths 

// holder for the SEQUENCE 
#define SEQUENCE_LAYERS 3 
byte SEQUENCE[SEQUENCE_LAYERS][ARRAY_SIZE]; //0=note 1=octave 2=velocity  
byte FROZEN_SEQUENCE[SEQUENCE_LAYERS][ARRAY_SIZE]; //0=note 1=octave 2=velocity  


// arrays to manage note length/endings 
byte NOTE_ON[VOICE_COUNT];
unsigned long NOTE_OFF[VOICE_COUNT];



/*
LOOKUP TABLES 
*/

//QUANTISATION 
byte MODES[MODE_COUNT][12]= {
  {0,1,2,3,4,5,6,7,8,9,10,11    },   // chromatic 
  {   0,2,4,5,7,9,11,12  }, //  major 
  { 0,2,4,5,8,9,11,12    }, // harmonic major 
  {  0,2,4,7,9,12,14   }, //  pentatonic major 
  { 0,2,3,5,7,8,10,12    }, // minor 
  { 0,2,3,5,7,8,11,12    }, // harmonic minor
  {  0,3,5,7,10,12,15,   }, // pentatonic minor  
  {  0,2,3,5,6,8,9,11   }, //  diminished
  {0,2,4,6,8,10,12    }, //whole tone 
  { 0,3,5,6,7,10,12    }, // blues 
  { 0,1,3,5,7,9,10,12    }, // javanese
  {0,2,4,5,6,8,10,12    }, // arabian 
  { 0,1,4,5,6,8,11,12    }, // persian 
  {  0,1,4,5,6,9,10,12   }, // oriental 
  {   0,1,4,5,7,8,10,12  }, // jewish 
  {   0,1,4,5,7,8,11,12  }, //  gypsy
  {   0,2,3,5,7,9,10,12  }, // dorian 
  {  0,2,4,5,7,8,11,12   }, // ethiopian 
};  

/* Rhythm system: 
0 = 4/4
1 = clock divisions 
2 = samba
3 = 3/4
4 = cuban
5 = bossanova 
*/

byte ODDS[ODDS_COUNT][ARRAY_SIZE]={
{
95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5
},{
95,5,95,5,95,5,95,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,5,5,5,5,95,5,5,5,5,5,5,5,95,5,5,5,5,5,5,5,95,5,5,5,5,5,5,5,95,95,95,95,95,95,95,95,95,5,95,5,95,5,95,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,5,5,5,5,95,5,5,5,5,5,5,5,95,5,5,5,5,5,5,5,95,5,5,5,5,5,5,5,95,95,95,95,95,95,95,95,
},{
95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,95,5,95,5,5,95,5,95,5,95,5,5,95,5,95,5,
},{
90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20
},{
95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,
},{
95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,  
}};



void setup() {
  //  Set MIDI baud rate:
Serial.begin(28800); // DEBUG 
  Serial1.begin(31250); // midi 
   randomSeed(analogRead(9));
 pinMode(redLED,OUTPUT);
 pinMode(greenLED1,OUTPUT);
 pinMode(greenLED2,OUTPUT);
 
}


/*
MAIN LOOP 
*/

void loop() {

byte changes;
byte loop_length = 16;

if(FILL == true){
randomiseValues();
fillRandom();
FILL = false;
}

else{

  for(int seqstep=0;seqstep<loop_length;seqstep++){   

TEMPO = map(analogRead(0),0,1024,MIN_TEMPO,MAX_TEMPO);
changes = map(analogRead(1),0,1024,0,ARRAY_SIZE); 
MODE_CHOICE = map(analogRead(2),0,1024,0,MODE_COUNT);
loop_length = (analogRead(3)/128)*8;

int velocity = getVelocity(SEQUENCE[2][seqstep]);
int notelength = TEMPO*random(10);

playNote(0x90, quantize(MODE_CHOICE, SEQUENCE[0][seqstep], SEQUENCE[1][seqstep]),velocity,notelength);
 

if (seqstep%4==0){digitalWrite(greenLED1, HIGH);}
else{digitalWrite(greenLED1, LOW);}
        
noteKill();
delay(TEMPO);
REPEAT_COUNT++;

// EVERY 96 NOTES, FREEZE AND REFILL  
  if(REPEAT_COUNT==96){  
    freeze();
    fillRandom();}
    if(REPEAT_COUNT == 192){
      unFreeze();
      REPEAT_COUNT = 0;};
}  


// MAKE CHANGES 

for (int i = 0; i<changes; i++){
byte changer=random(ARRAY_SIZE); // choose an entry to change 
if(random(DENSITY)<ODDS[ODDS_CHOICE][changer] && SEQUENCE[2][changer]>0){ // if a rhythmically interesting note and non-silent 
SEQUENCE[0][changer] = SEQUENCE[0][changer]+(random(3)-1);}}


} 
}


/*
FUNCTIONS 
*/

// midi note player 

void playNote(byte cmd, byte pitch, byte velocity, int duration) {
unsigned long time = millis();
// find an empty note slot 
boolean found_slot = false;
for (int i=0;i<VOICE_COUNT;i++){
if (NOTE_ON[i] == 0){
  
  NOTE_ON[i] = pitch;
  NOTE_OFF[i] = time+duration;
  found_slot = true;
  
  break;
}
}

// RED LIGHT SHOWS NOTE BUFFER IS OVERFULL 
if (found_slot == false){digitalWrite (redLED, HIGH);}
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
for(int seqstep=0;seqstep<ARRAY_SIZE;seqstep++){
    byte newnote=random(7);
    byte newoctave = random(5)+3;
    if(random(DENSITY)<ODDS[ODDS_CHOICE][seqstep]){
    SEQUENCE[0][seqstep]= newnote; // note 
    SEQUENCE[1][seqstep]= newoctave;  // octave 
    SEQUENCE[2][seqstep]= random(ODDS[ODDS_CHOICE][seqstep]+27); // velocity 
}
}
}

// READ VELOCITY POTS AND RETURN CORRECT VELOCITY 
int getVelocity(int velocity){
byte minvelocity = map(analogRead(4),0,1024,1,127);
byte maxvelocity = map(analogRead(5),0,1024,minvelocity,127);
if (velocity>0){
if (velocity<=minvelocity){velocity=minvelocity;};
if (velocity>=maxvelocity){velocity=maxvelocity;};
}
return velocity;
}

void randomiseValues(){
 MODE_CHOICE=random(MODE_COUNT);
ODDS_CHOICE = random(ODDS_COUNT);
DENSITY = (random(MAX_DENSITY-MIN_DENSITY))+MIN_DENSITY; 
LONG_SHORT = random(16);
BASE_TIME = random(16); 
}



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

Experiments to try: 
        Create change systems for note/velocity/duration and attach them to knobs 


	
 */



/*
GLOBAL VARIABLES 
*/
byte MODE_CHOICE;
#define MODE_COUNT 14

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

byte LONG_SHORT;
byte BASE_time; // multiplier for note lengths 

// holder for the SEQUENCE 
byte SEQUENCE[3][ARRAY_SIZE]; //0=note 1=octave 2=velocity  

// arrays to manage note length/endings 
byte NOTE_ON[VOICE_COUNT];
unsigned long NOTE_OFF[VOICE_COUNT];



/*
LOOKUP TABLES 
*/

//QUANTISATION 
byte MODES[MODE_COUNT][12]= {
  {0,1,2,3,4,5,6,7,8,9,10,11    },   // chromatic 
  {0,2,4,6,8,10,12    }, //whole tone 
  { 0,3,5,6,7,10,12    }, // blues 
  { 0,1,3,5,7,9,10,12    }, // javanese
  { 0,2,3,5,7,8,10,12    }, // minor 
  { 0,2,3,5,7,8,11,12    }, // harmonic minor 
  {0,2,4,5,6,8,10,12    }, // arabian 
  { 0,2,4,5,8,9,11,12    }, // harmonic major 
  { 0,1,4,5,6,8,11,12    }, // persian 
  {  0,1,4,5,6,9,10,12   }, // oriental 
  {   0,1,4,5,7,8,10,12  }, // jewish 
  {   0,1,4,5,7,8,11,12  }, //  gypsy
  {   0,2,3,5,7,9,10,12  }, // dorian 
  {  0,2,4,5,7,8,11,12   }, // ethiopian 
 
//  {     }, //  
  
  
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
   randomSeed((analogRead(9)+analogRead(10)+analogRead(11))/3);
 pinMode(redLED,OUTPUT);
 pinMode(greenLED1,OUTPUT);
 pinMode(greenLED2,OUTPUT);
 
}


/*
MAIN LOOP 
*/

void loop() {


  byte MODESchange;
byte playsteps = 16; // how many MODES to play in each loop 


// FIRST PLAY, FILL UP THE SEQUENCE  
if(FILL == true){

  // CHOOSE INITIAL VARIABLES 
  
  // Random or fixed scale selection 
MODE_CHOICE=random(MODE_COUNT);
//MODE_CHOICE=3;

// Random or fixed rhythm selection 
ODDS_CHOICE = random(ODDS_COUNT);
//MODE_CHOICE = 4;

// select density
DENSITY = (random(MAX_DENSITY-MIN_DENSITY))+MIN_DENSITY;

//EXPERIMENTAL 
// Choose divider for long/short stresses 
LONG_SHORT = random(16);
BASE_time = random(16);


// FILL THE LOOP 
for(int seqstep=0;seqstep<ARRAY_SIZE;seqstep++){
 
    byte newnote=random(7);
    byte newoctave = random(5)+3;
    if(random(DENSITY)<ODDS[ODDS_CHOICE][seqstep]){
    SEQUENCE[0][seqstep]= newnote; // note 
    SEQUENCE[1][seqstep]= newoctave;  // octave 
    SEQUENCE[2][seqstep]= random(ODDS[ODDS_CHOICE][seqstep]+27); // velocity 

}
}
FILL = false;
}



else{

// AFTER FIRST FILL, STEP THROUGH THE SEQUENCE  
  
for(int seqstep=0;seqstep<playsteps;seqstep++){    
byte randomNote = 0;

// ROUTINES TO CHECK THE KNOBS  
// pot zero = TEMPO 
TEMPO = map(analogRead(0),0,1024,MIN_TEMPO,MAX_TEMPO);
 
 // Knob 2
 
  // RANDOMISE KNOB 
//  if (analogRead(2)>random(1024)){
//   randomNote = -2+random(4);
//  if (DEBUG==true){Serial.println("TWEAK");}; 
//  }
//  
// SCALE SELECTION KNOB 
  MODE_CHOICE = map(analogRead(2),0,1024,0,MODE_COUNT);
 
// byte octave_override = map(analogRead(2),0,1024,0,10)+2;
//  byte note_override = map(analogRead(2),0,1024,0,7);

 
 // VELOCITY HIGH/LOW KNOBS 
 byte velocity = SEQUENCE[2][seqstep];
 if (velocity>0){
  byte minvelocity = map(analogRead(4),0,1024,1,127);
  byte maxvelocity = map(analogRead(5),0,1024,minvelocity,127);
 if (velocity<=minvelocity){velocity=minvelocity;};
if (velocity>=maxvelocity){velocity=maxvelocity;};
 }

// // CHANGE KNOB 
MODESchange = map(analogRead(1),0,1024,0,ARRAY_SIZE); 


 
 // LOOP KNOB 
 playsteps = (analogRead(3)/128)*8;
 
 // PLAY THE NOTE 
 
 // TEMPOrarily attach note duration to pot 3 for testing, or to TEMPO 
// int notelength = analogRead(3);
// int notelength = TEMPO;
// int notelength = TEMPO+(analogRead(3));
// int notelength = seqstep*10;
//int notelength = (127-velocity)*10;
//int notelength = (127-SEQUENCE[0][seqstep])*10;
//int notelength = TEMPO*2; 
int notelength; 
if (seqstep % LONG_SHORT == 0){notelength = random(8)*TEMPO;} // play on specific note divisions 
else {notelength = TEMPO*BASE_time;};

 
 
  playNote(0x90, quantize(MODE_CHOICE, SEQUENCE[0][seqstep], SEQUENCE[1][seqstep])+randomNote,velocity,notelength);
 
 // OCTAVE OVERRIDE VERSION 
//   playNote(0x90, quantize(MODE_CHOICE, SEQUENCE[0][seqstep], octave_override)+randomNote,velocity,notelength);
       
 // OCTAVE OVERRIDE VERSION 
//   playNote(0x90, quantize(MODE_CHOICE, note_override, SEQUENCE[1][seqstep])+randomNote,velocity,notelength);
 


//  if (DEBUG == true){Serial.print("TEMPO=");Serial.println(TEMPO);};

if (seqstep%4==0){
digitalWrite(greenLED1, HIGH);
}
else{
digitalWrite(greenLED1, LOW);
}
        
 noteKill();
delay(TEMPO);

}  


// MAKE CHANGES 

for (int i = 0; i<MODESchange; i++){
byte changer=random(ARRAY_SIZE); // choose an entry to change 
if(random(DENSITY)<ODDS[ODDS_CHOICE][changer] && SEQUENCE[2][changer]>0){ // if a rhythmically interesting note and non-silent 
SEQUENCE[0][changer] = SEQUENCE[0][changer]+(random(3)-1);

}
}

//for (int i = 0; i<MODESchange; i++){
//byte changer=random(ARRAY_SIZE);
//byte a=random(ODDS[ODDS_CHOICE][changer])/14;
//noteplay=quantize(MODE_CHOICE, a, random(4)+4);
//if(random(density)<ODDS[ODDS_CHOICE][changer]){
//SEQUENCE[0][changer]= noteplay;
//SEQUENCE[1][changer]= random(ODDS[ODDS_CHOICE][changer]+27);
//}
//}



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






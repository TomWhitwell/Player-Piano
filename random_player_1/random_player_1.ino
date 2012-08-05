/*
Random piano sequencer 
 
 Notes on this version: 
 - sequencer looping
 
 Knobs: 
 0 = tempo
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
Create standard internal numbering system (i.e. 0-256) that can be applied to any variable.
	ie create a LFO which can be applied to note/velocity/duration 
	build markov chain around this variable
		so you can repeat/learn velocity chains as easily as note chains 
	BUT: How to connect 0-256 number with 0-7+0-8 note+octave system? 
System to change a particular global variable for a period 
	i.e. change base note / mode / tempo for n bars, then revert 
	Is it a temporary change vs a permanent change - maybe with a buffer to reverse the change? 
Rework rhythm system - enable changes of rhythm 
	(Could be covered by note change function above, i.e. ChangeVelocityByRhythm) 


	
 */



/*
GLOBAL VARIABLES 
*/
byte modechoice;
byte noteplay;
#define arraysize 128
#define modecount 14
int tempo  = 60;
int maxtempo=0;
int mintempo = 200;
boolean fill=true;
boolean debug = false; 
boolean Display = true; 
#define greenLED2 44
#define greenLED1 45
#define redLED 46
#define numvoice 16 // maximum polyphony 
#define density 100 // 100 = average, 200 = low, 50 = high 
#define oddsCount 6
byte oddsChoice;
byte LongShort;
byte BaseTime =2;

/*
LOOKUP TABLES 
*/

//QUANTISATION 
byte notes[modecount][12]= {
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

byte odds[oddsCount][arraysize]={
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

// holder for the sequence 
byte sequence[3][arraysize]; //0=note 1=octave 2=velocity  

// arrays to manage note length/endings 
byte onNote[numvoice];
unsigned long offTime[numvoice];
unsigned long time;

void setup() {
  //  Set MIDI baud rate:
Serial.begin(28800); // debug 
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


  byte noteschange;
byte playsteps = 16; // how many notes to play in each loop 


// FIRST PLAY, FILL UP THE SEQUENCE  
if(fill == true){

  // CHOOSE INITIAL VARIABLES 
  
  // Random or fixed scale selection 
modechoice=random(modecount);
//modechoice=3;

// Random or fixed rhythm selection 
oddsChoice = random(oddsCount);
//modechoice = 4;


//EXPERIMENTAL 
// Choose divider for long/short stresses 
LongShort = random(16);
BaseTime = random(8);


// FILL THE LOOP 
for(int seqstep=0;seqstep<arraysize;seqstep++){
 
    byte newnote=random(7);
    byte newoctave = random(5)+3;
    if(random(density)<odds[oddsChoice][seqstep]){
    sequence[0][seqstep]= newnote; // note 
    sequence[1][seqstep]= newoctave;  // octave 
    sequence[2][seqstep]= random(odds[oddsChoice][seqstep]+27); // velocity 

}
}
fill = false;
}



else{

// AFTER FIRST FILL, STEP THROUGH THE SEQUENCE  
  
for(int seqstep=0;seqstep<playsteps;seqstep++){    
byte randomNote = 0;

// ROUTINES TO CHECK THE KNOBS  
// pot zero = tempo 
tempo = map(analogRead(0),0,1024,mintempo,maxtempo);
 
 // Knob 2
 
  // RANDOMISE KNOB 
//  if (analogRead(2)>random(1024)){
//   randomNote = -2+random(4);
//  if (debug==true){Serial.println("TWEAK");}; 
//  }
//  
// SCALE SELECTION KNOB 
  modechoice = map(analogRead(2),0,1024,0,modecount);
 
// byte octave_override = map(analogRead(2),0,1024,0,10)+2;
//  byte note_override = map(analogRead(2),0,1024,0,7);

 
 // VELOCITY HIGH/LOW KNOBS 
 byte velocity = sequence[2][seqstep];
 if (velocity>0){
  byte minvelocity = map(analogRead(4),0,1024,1,127);
  byte maxvelocity = map(analogRead(5),0,1024,minvelocity,127);
 if (velocity<=minvelocity){velocity=minvelocity;};
if (velocity>=maxvelocity){velocity=maxvelocity;};
 }

// // CHANGE KNOB 
noteschange = map(analogRead(1),0,1024,0,arraysize); 


 
 // LOOP KNOB 
 playsteps = (analogRead(3)/128)*8;
 
 // PLAY THE NOTE 
 
 // temporarily attach note duration to pot 3 for testing, or to tempo 
// int notelength = analogRead(3);
// int notelength = tempo;
// int notelength = tempo+(analogRead(3));
// int notelength = seqstep*10;
//int notelength = (127-velocity)*10;
//int notelength = (127-sequence[0][seqstep])*10;
//int notelength = tempo*2; 
int notelength; 
if (seqstep % LongShort == 0){notelength = random(8)*tempo;} // play on specific note divisions 
else {notelength = tempo*BaseTime;};

 
 
  noteOn(0x90, quantize(modechoice, sequence[0][seqstep], sequence[1][seqstep])+randomNote,velocity,notelength);
 
 // OCTAVE OVERRIDE VERSION 
//   noteOn(0x90, quantize(modechoice, sequence[0][seqstep], octave_override)+randomNote,velocity,notelength);
       
 // OCTAVE OVERRIDE VERSION 
//   noteOn(0x90, quantize(modechoice, note_override, sequence[1][seqstep])+randomNote,velocity,notelength);
 


//  if (debug == true){Serial.print("tempo=");Serial.println(tempo);};

if (seqstep%4==0){
digitalWrite(greenLED1, HIGH);
}
else{
digitalWrite(greenLED1, LOW);
}
        
 noteKill();
delay(tempo);

}  


// MAKE CHANGES 

for (int i = 0; i<noteschange; i++){
byte changer=random(arraysize); // choose an entry to change 
if(random(density)<odds[oddsChoice][changer] && sequence[2][changer]>0){ // if a rhythmically interesting note and non-silent 
sequence[0][changer] = sequence[0][changer]+(random(3)-1);

}
}

//for (int i = 0; i<noteschange; i++){
//byte changer=random(arraysize);
//byte a=random(odds[oddsChoice][changer])/14;
//noteplay=quantize(modechoice, a, random(4)+4);
//if(random(density)<odds[oddsChoice][changer]){
//sequence[0][changer]= noteplay;
//sequence[1][changer]= random(odds[oddsChoice][changer]+27);
//}
//}



} 

}


/*
FUNCTIONS 
*/

// midi note player 

void noteOn(byte cmd, byte pitch, byte velocity, int duration) {
time = millis();
// find an empty note slot 
boolean foundslot = false;
for (int i=0;i<numvoice;i++){
if (onNote[i] == 0){
  
  onNote[i] = pitch;
  offTime[i] = time+duration;
  foundslot = true;
  
  break;
}
}

// RED LIGHT SHOWS NOTE BUFFER IS OVERFULL 
if (foundslot == false){digitalWrite (redLED, HIGH);}
else{digitalWrite (redLED, LOW);};
  
  Serial1.write(cmd);
  Serial1.write(pitch);
  Serial1.write(velocity);

  
  if (debug == true && velocity>0){  
  Serial.print("Pitch ");
  Serial.print(pitch);
  Serial.print(" Velocity ");
  Serial.println(velocity);
  }
}

// Check for held notes that should be killed, and kill them. 

void noteKill(){
time=millis();
for (int i=0;i<numvoice;i++){
if (onNote[i]>0 && offTime[i]<time){  // if note value>0 and note off time < time
 
 byte pitch = onNote[i];
 byte velocity = 0;
  Serial1.write(0x90);
  Serial1.write(pitch);
  Serial1.write(velocity);
  
      onNote[i]=0; //wipe onNote entry to signal no active note
}}}


// Quantizer 

byte quantize(int scale, int note, int octave){
  return octave*12+notes[scale][note]; 
}






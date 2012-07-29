/*
Random piano sequencer 
 
 Notes on this version: 
 - sequencer looping
 
 Knobs: 
 0 = tempo
 3 = tweaks +/- 2 notes on playback, positiion = probability 
 5 & 6 = min and max velocity
 
 
Schematic in pot_box_schematic 
 
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
#define greenLED2 44
#define greenLED1 45
#define redLED 46
#define numvoice 16 // maximum polyphony 
#define density 100 // 100 = average, 200 = low, 50 = high 
#define oddsCount 6
byte oddsChoice;

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
2 = no rhythm 
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
50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
},{
90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20
},{
95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,95,5,95,95,5,95,95,5,
},{
95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,95,5,5,95,5,5,95,5,5,5,95,5,5,95,5,5,  
}};

// holder for the sequence 
byte sequence[2][arraysize];

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


  


// FIRST PLAY, FILL UP THE SEQUENCE  
if(fill == true){

  // Random or fixed scale selection 
modechoice=random(modecount);
//modechoice=3;

// Random or fixed rhythm selection 
//oddsChoice = random(oddsCount);
modechoice = 4;

for(int seqstep=0;seqstep<arraysize;seqstep++){    
byte a=random(odds[oddsChoice][seqstep])/14;
       noteplay=quantize(modechoice, a, random(4)+4);
if(random(density)<odds[oddsChoice][seqstep]){
sequence[0][seqstep]= noteplay;
sequence[1][seqstep]= random(odds[oddsChoice][seqstep]+27);

}
}
fill = false;
}



else{

// AFTER FIRST FILL, STEP THROUGH THE SEQUENCE  
  
for(int seqstep=0;seqstep<arraysize;seqstep++){    
byte randomNote = 0;

// ***check pots 
// pot zero = tempo 
tempo = map(analogRead(0),0,1024,mintempo,maxtempo);
  
  // randomise notes, outside the quantisation, with pot 2
  if (analogRead(2)>random(1024)){
   randomNote = 2+random(4);
  if (debug==true){Serial.println("TWEAK");}; 
  }
 
 
 byte velocity = sequence[1][seqstep];
 if (velocity>0){
  byte minvelocity = map(analogRead(4),0,1024,1,127);
  byte maxvelocity = map(analogRead(5),0,1024,minvelocity,127);
 if (velocity<=minvelocity){velocity=minvelocity;};
if (velocity>=maxvelocity){velocity=maxvelocity;};
 }
 
 // PLAY THE NOTE 
 
 // temporarily attach note duration to pot 3 for testing, or to tempo 
// int notelength = analogRead(3);
// int notelength = tempo;
 int notelength = tempo+(analogRead(3));
// int notelength = seqstep*10;
//int notelength = (127-velocity)*10;
//int notelength = (127-sequence[0][seqstep])*10;
//int notelength = tempo*2; 

 
 
  noteOn(0x90, sequence[0][seqstep]+randomNote,velocity,notelength);
        


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

byte changer=random(arraysize);
byte a=random(odds[oddsChoice][changer])/14;
noteplay=quantize(modechoice, a, random(4)+4);
sequence[0][changer]= noteplay;
sequence[1][changer]= random(odds[oddsChoice][changer]+27);
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






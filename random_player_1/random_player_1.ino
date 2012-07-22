/*
Random piano sequencer 
 
 Notes on this version: 
 - sequencer looping
 
 Knobs: 
 
 0 = tempo
 3 = tweaks +/- 2 notes on playback, positiion = probability 
 5 & 6 = min and max velocity
 
 
 Schematic:  
 
 
 */



/*
GLOBAL VARIABLES 
*/
byte modechoice;
byte noteplay;
#define arraysize 128
#define modecount 8
int tempo  = 60;
int maxtempo=0;
int mintempo = 200;
boolean fill=true;
boolean debug = true; 

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
};  

byte odds[arraysize]={
//100,0,0,0,100,0,0,0,100,0,0,0,100,0,0,0
//90,10,10,10,90,10,10,10,90,10,10,10,90,10,10,10,90,10,10,10,90,10,10,10,90,10,10,10,90,10,10,10
//95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5
95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5



//50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
//100,95,90,85,80,75,70,65,60,50,45,40,35,30,25,20
//100,90,80,70,5,5,5,5,5,5,5,5,5,5,5,40,80,10,10,0
//90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20
};

byte sequence[2][arraysize];

void setup() {
  //  Set MIDI baud rate:
Serial.begin(28800); // debug 
  Serial1.begin(31250); // midi 
   randomSeed(analogRead(9));
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

for(int seqstep=0;seqstep<arraysize;seqstep++){    
byte a=random(odds[seqstep])/14;
       noteplay=quantize(modechoice, a, random(4)+4);
if(random(100)<odds[seqstep]){
sequence[0][seqstep]= noteplay;
sequence[1][seqstep]= random(odds[seqstep]+27);

}
}
fill = false;
}



else{

// AFTER FIRST FILL, STEP THROUGH THE SEQUENCE  
  
for(int seqstep=0;seqstep<arraysize;seqstep++){    
        noteOn(0x90, sequence[0][seqstep],sequence[1][seqstep]);
        
          tempo = map(analogRead(0),0,1024,mintempo,maxtempo);
//  if (debug == true){Serial.print("tempo=");Serial.println(tempo);};
        
delay(tempo);
}  

byte changer=random(arraysize);
byte a=random(odds[changer])/14;
noteplay=quantize(modechoice, a, random(4)+4);
sequence[0][changer]= noteplay;
sequence[1][changer]= random(odds[changer]+27);
}
 
noteOn(0x90,quantize(modechoice, random(8),random(4)+4),0);
  
}


/*
FUNCTIONS 
*/

// midi note player 

//  plays a MIDI note.  Doesn't check to see that
//  cmd is greater than 127, or that data values are  less than 127:
void noteOn(int cmd, int pitch, int velocity) {
  
  // note scale break, with pot 2
  if (analogRead(2)>random(1024) && velocity>0){
   pitch = pitch-2+random(4);
  if (debug==true){Serial.println("TWEAK");}; 
  }

if (velocity>0){  
  byte minvelocity = map(analogRead(4),0,1024,1,127);
  byte maxvelocity = map(analogRead(5),0,1024,minvelocity,127);
    if (debug==true){
    Serial.print("minvelocity=");
    Serial.print(minvelocity);
    Serial.print(" maxvelocity=");
    Serial.println(maxvelocity);
    
    Serial.print("pre_velocity=");Serial.print(velocity);}; 
if (velocity<=minvelocity){velocity=minvelocity;};
if (velocity>=maxvelocity){velocity=maxvelocity;};
    if (debug==true){Serial.print(" post_velocity=");Serial.println(velocity);}; 
}
  
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

// Quantizer 

byte quantize(int scale, int note, int octave){
  return octave*12+notes[scale][note]; 
}






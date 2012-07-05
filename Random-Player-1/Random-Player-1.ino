/*
Random piano sequencer 
 
 Notes on this version: 
 - sequencer looping
 
 
 Schematic:  
 
 
 */



/*
GLOBAL VARIABLES 
*/
byte modechoice;
byte noteplay;
#define arraysize 128
#define tempo 60
boolean fill=true;


/*
LOOKUP TABLES 
*/

//QUANTISATION 
byte notes[8][12]= {
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
// Serial.begin(9600); // debug 
  Serial.begin(31250); // midi 
   randomSeed(analogRead(0));
}


/*
MAIN LOOP 
*/

void loop() {



if(fill == true){
modechoice=random(7);
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

for(int seqstep=0;seqstep<arraysize;seqstep++){    
        noteOn(0x90, sequence[0][seqstep],sequence[1][seqstep]);
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
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}

// Quantizer 

byte quantize(int scale, int note, int octave){
  return octave*12+notes[scale][note]; 
}





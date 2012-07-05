
/*



*/

#define chain_size 7
#define max_count 25000
int markov[chain_size*chain_size][chain_size];
byte oldnote;
byte oldernote;
byte newnote=random(chain_size); 
int stepcount;
byte modechoice;
byte noteplay;
#define arraysize 16
#define tempo 200
#define seqlength 16
boolean fill=true;
byte seqstep;
boolean think=true;
int think_time = 20000;
int increment=5; // how many the count goes up in the chain each time a note is played 
/*
LOOKUP TABLES 
*/

//QUANTISATION 
byte notes[8][12]= {
  {0,1,2,3,4,5,6,7,8,9,10,11    },   // chromatic 
  {0,2,4,6,8,10,12,14    }, //whole tone 
  { 0,3,5,6,7,10,12,15    }, // blues 
  { 0,1,3,5,7,9,10,12,13    }, // javanese
  { 0,2,3,5,7,8,10,12,14    }, // minor 
  { 0,2,3,5,7,8,11,12,14    }, // harmonic minor 
  {0,2,4,5,6,8,10,12,14    }, // arabian 
  { 0,2,4,5,8,9,11,12,14    }, // harmonic major 
};  

byte odds[arraysize]={
100,0,0,0,100,0,0,0,100,0,0,0,100,0,0,0
//90,10,10,10,90,10,10,10,90,10,10,10,90,10,10,10,90,10,10,10,90,10,10,10,90,10,10,10,90,10,10,10
//95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5
//95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5,95,5,5,5
//50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
//100,95,90,85,80,75,70,65,60,50,45,40,35,30,25,20
//100,90,80,70,5,5,5,5,5,5,5,5,5,5,5,40,80,10,10,0
//90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,90,20,20,20,20
};

byte sequence[2][arraysize];

void setup() {
 Serial.begin(9600); // debug 
//  Serial.begin(31250); // midi 
  
  fillchain();
randomSeed(analogRead(0));
modechoice=1;
}

void loop() {

  if(think==true){
   for(int i=0;i<think_time;i++){
      oldernote=oldnote;
  oldnote=newnote; 
//newnote=random(chain_size);
newnote = choose_note(oldnote,oldernote);
update_chain(newnote,oldnote,oldernote);

   } 
    think=false; 
  }
  
  
//  if (random(100)<odds[seqstep]){
  
  
  
  oldernote=oldnote;
  oldnote=newnote; 
//newnote=random(chain_size);
newnote = choose_note(oldnote,oldernote);
update_chain(newnote,oldnote,oldernote);

noteOn(0x90, quantize(modechoice,newnote,5),30+random(60));


//if(newnote==1 && oldnote == 3){
//delay(10);
//  noteOn(0x90, quantize(modechoice,newnote,7),60+random(60));
//  noteOn(0x90, quantize(modechoice,newnote,6),60+random(60));
// 
//}
//
//if(random(100)<20){
//delay(10);
//  noteOn(0x90, quantize(modechoice,oldernote,3),10+random(60));
//}
//
//
//if(random(100)<50){
//delay(10);
//  noteOn(0x90, quantize(modechoice,oldernote,5),0);
//}
//
//



seqstep++;
//  }
delay(tempo);




if (seqstep>seqlength){
 seqstep=0; 
}

//if(stepcount%200==0){
//Serial.print("Step ");
//Serial.println(stepcount);
//drawchain();
//delay(1000);
//}
//Serial.print(newnote);
//Serial.print(",");
//
//
//
//if (stepcount>20000){
// delay(100000); 
//}
//  stepcount++;
}




void drawchain(){
for(int x=0; x<chain_size*chain_size; x++){
 for (int y = 0; y< chain_size; y++){
     Serial.print(markov[x][y]);
   Serial.print(",");
 }
 Serial.println(" ");
}
Serial.println("___________________");   
}


void fillchain(){
for(int x=0; x<chain_size*chain_size; x++){
 for (int y = 0; y< chain_size; y++){
markov[x][y]=1;
 }
}
}

void update_chain(byte newer, byte older, byte oldest){
  if(markov[oldest*chain_size+older][newer]<max_count){
//    markov[oldest*chain_size+older][newer]++;
markov[oldest*chain_size+older][newer] = markov[oldest*chain_size+older][newer]+increment;

  }
}

byte choose_note(byte last_note,byte note_before){
  int total=0;
  for (byte y=0;y<chain_size;y++){
   total=total+markov[note_before*chain_size+last_note][y]; 
  }
int running_total=0;
int random_number = random(total+1);
for( byte i=0; i<chain_size; i++){
 running_total=running_total+markov[note_before*chain_size+last_note][i];
  if (random_number<=running_total){
 /*
    // DEBUG ROUTINE 
    
if (stepcount%50==0){
 Serial.print("Choose_note routine: Total=");
 Serial.print(total);
Serial.print(" Running_total=");
Serial.print(running_total);
Serial.print(" Random_number=");
Serial.print(random_number);
Serial.print(" result=");
Serial.println(i);
}

*/
    return i;
    break;
  }
}
  }
  
  
  

/*
FUNCTIONS 
*/

// midi note player 

//  plays a MIDI note.  Doesn't check to see that
//  cmd is greater than 127, or that data values are  less than 127:
void noteOn(int cmd, int pitch, int velocity) {
//  Serial.write(cmd);
//  Serial.write(pitch);
//  Serial.write(velocity);
}

// Quantizer 

byte quantize(int scale, int note, int octave){
  Serial.print("Quantize input: ");
  Serial.print(note);
  Serial.print(" output: ");
  Serial.println(notes[scale][note]);
  return octave*12+notes[scale][note]; 

  
}



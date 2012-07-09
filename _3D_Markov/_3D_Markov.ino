
/*

Working first order markov chain
next - add multiple orders using 3d arrays 

Prototype for third order chain 

*/


// number of possible states 
#define chain_size 3
// number of orders of chain - this code suggests 
#define chain_order 3
#define max_count 30000
byte chain_initial_state=1;

// one more dimension than the chain size 
// if max_count > 256, use int, otherwise use byte 
int markov[chain_size][chain_size][chain_size][chain_size];

//Holder for notes in play, 
byte note_store[chain_size+1];


int stepcount;


void setup() {

Serial.begin(115200);

// fills chains with something at setup  
fillchain(chain_initial_state);

//add random notes to start of chain;
for (int i=0;i<chain_order;i++){
 note_store[i]=random(chain_size); 
}


randomSeed(analogRead(0));


}

void loop() {


  
  
  
  
update_chain(newnote,oldnote);
if(stepcount%2000==0){
Serial.print("Step ");
Serial.println(stepcount);
drawchain();
}

if (stepcount>20000){
 delay(100000); 
}
  stepcount++;
}




void drawchain(){
  for(int z=0;z<chain_order;z++){
for(int x=0; x<chain_size; x++){
 for (int y = 0; y< chain_size; y++){
     Serial.print(markov[x][y][z]);
   Serial.print(",");
 }
 Serial.println(" ");
}
Serial.println("--end of order--");
}
Serial.println("__end of iteration__");   
}


void fillchain(byte chain_initial_content){
for(int q=0;q<chain_size;q++){
for(int z=0;z<chain_size;z++){
for(int x=0; x<chain_size; x++){
 for (int y = 0; y< chain_size; y++){
markov[q][x][y][z]=chain_initial_content;
 }
}
    }
}
}

void update_chain(byte newer, byte older){
  if(markov[older][newer]<max_count){
    markov[older][newer]++;
  }
}

byte choose_note(byte last_note){
  int total=0;
  for (byte y=0;y<chain_size;y++){
   total=total+markov[last_note][y]; 
  }



int running_total=0;
int random_number = random(total+1);
for( byte i=0; i<chain_size; i++){
 running_total=running_total+markov[last_note][i];
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

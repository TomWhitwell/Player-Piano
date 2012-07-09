
/*

Working third order chain, but 

*/


// number of possible states 
#define chain_size 6
// number of orders of chain - this code suggests 
#define chain_order 3
#define max_count 256
byte multiplier = 2; // how much is added each time, defines strength of chain 
byte chain_initial_state=1;

// one more dimension than the chain size 
// if max_count > 256, use int, otherwise use byte 
byte markov[chain_size][chain_size][chain_size][chain_size];

//Holder for notes in play, 
byte note_store[chain_size+1];


int stepcount;


void setup() {
randomSeed(analogRead(0));

Serial.begin(115200);

// fills chains with something at setup  
fillchain(chain_initial_state);

//add random notes to start of note store, to input to chain;
Serial.print ("Init notes ");
for (int i=0;i<chain_order;i++){
 note_store[i]=random(chain_size); 
Serial.print (note_store[i]);
Serial.print (" ");
}
Serial.println (" ");
Serial.print ("Generated notes:");





}

void loop() {

// add a new chain-generated note to the end of the note store 
note_store[3] = choose_note(note_store[0],note_store[1],note_store[2]);

// print the new note 
Serial.print(note_store[3]);
Serial.print(" ");
if (note_store[3] == 0){Serial.println("");};

// update the chain 
update_chain(note_store[0],note_store[1],note_store[2],note_store[3]);

// progress the note store
for (int i=0;i<chain_size;i++){
 note_store[i] = note_store[i+1]; 
}

delay(20);
//if (stepcount>4){
// Serial.println(" ");
//stepcount = 0; 
//}
  stepcount++;
}



//// DRAW OUT THE CHAIN - NEEDS FIXING 
//void drawchain(){
//  for(int z=0;z<chain_order;z++){
//for(int x=0; x<chain_size; x++){
// for (int y = 0; y< chain_size; y++){
//     Serial.print(markov[x][y][z]);
//   Serial.print(",");
// }
// Serial.println(" ");
//}
//Serial.println("--end of order--");
//}
//Serial.println("__end of iteration__");   
//}


// FILL THE CHAIN WITH INITIAL CONTENT - ONLY WORKS WITH 3RD GEN CHAINS 
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

// ADD THE NEW NOTE TO THE CHAIN 
void update_chain(byte a, byte b, byte c, byte d){
  if(markov[a][b][c][d]<max_count){
    markov[a][b][c][d] = markov[a][b][c][d]+multiplier;
  }
  else{Serial.print("x");}
}

// LET THE CHAIN CHOOSE A NEW NOTE 
byte choose_note(byte a, byte b, byte c){
  
//  count the total hits in the relevant chain 
  int total=0;
  for (byte y=0;y<chain_size;y++){
   total=total+markov[a][b][c][y]; 
  }


// pick and return a new number, based on weighted dice throw 
int running_total=0;
int random_number = random(total+1);
for( byte i=0; i<chain_size; i++){
 running_total=running_total+markov[a][b][c][i];
  if (random_number<=running_total){
    return i;
    break;
  }
}
  
}


/*

Working first order markov chain

next - add multiple orders using 3d arrays 


*/

#define chain_size 7
#define max_count 30000
int markov[chain_size][chain_size];
byte oldnote; 
byte newnote=random(chain_size); 
int stepcount;


void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
fillchain();
randomSeed(analogRead(0));
}

void loop() {
oldnote=newnote; 
//newnote=random(chain_size);
newnote = choose_note(oldnote);
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
for(int x=0; x<chain_size; x++){
 for (int y = 0; y< chain_size; y++){
     Serial.print(markov[x][y]);
   Serial.print(",");
 }
 Serial.println(" ");
}
Serial.println("___________________");   
}


void fillchain(){
for(int x=0; x<chain_size; x++){
 for (int y = 0; y< chain_size; y++){
markov[x][y]=1;
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



#define greenLED2 44
#define greenLED1 45
#define redLED 46
boolean firstRun = true;
byte sensorValue[88];


void setup() {
    Serial.begin(115200); // DEBUG 
//    Serial.println("**************");
    Serial1.begin(31250); // midi 
    randomSeed(analogRead(9));
    pinMode(redLED,OUTPUT);
    pinMode(greenLED1,OUTPUT);
    pinMode(greenLED2,OUTPUT);
pedalDown();

}

// the loop routine runs over and over again forever:
void loop() {

  int DELAY = analogRead(0)*4;
  byte SPREAD = map(analogRead(1),0,1024,0,88); 
  byte CENTRE = map(analogRead(2),0,1024,0,88);
byte NOTE_DENSITY = map(analogRead(3),0,1024,0,88);
byte MIN_VELOCITY = map(analogRead(4),0,1024,1,127);
byte MAX_VELOCITY = map(analogRead(5),0,1024,MIN_VELOCITY,127);
Serial.println(CENTRE+(random(SPREAD)-(SPREAD/2)));


if (firstRun == true){
  for (int i=0;i<=88;i++){
sensorValue[i] = 0;
}
for (int i=0;i<NOTE_DENSITY;i++){
  byte newNote = 44+(random(SPREAD)-(SPREAD/2));
  if (newNote<88 && newNote>0){
  sensorValue[newNote] = map(random(127),0,127,MIN_VELOCITY,MAX_VELOCITY);}
}
firstRun = false; 
}

for (int i=0;i<NOTE_DENSITY;i++){
  byte newNote = 44+(random(SPREAD)-(SPREAD/2));
  if (newNote<88 && newNote>0){
  sensorValue[newNote] = map(random(127),0,127,MIN_VELOCITY,MAX_VELOCITY);}
  sensorValue[newNote+1] = 0;

}


  for (int i=0;i<=88;i++){
if(sensorValue[i]>0){
  playNote(0x90, i+20,sensorValue[i]); 
}
Serial.print(sensorValue[i]);
} 

Serial.println(" ");
int actualDelay = DELAY+(random(DELAY/10)-DELAY/20);
if (actualDelay <1){actualDelay = 1;};
delay(actualDelay);
}


void playNote(byte cmd, byte pitch, byte velocity) {
  if (pitch>21 && pitch <106){
  Serial1.write(cmd);
  Serial1.write(pitch);
  Serial1.write(velocity);}
  
//   
//    
//  Serial.print(pitch);
//  Serial.print (" | ");
//  Serial.println(velocity);
//  
  
}


void pedalDown(){
 byte status_byte = 176; // control change on channel 1
byte cc_number=64; // pedal control 
byte data_send = 127; // pedal DOWN
Serial1.print(status_byte); // send status byte
Serial1.print(cc_number); // send CC number
Serial1.print(data_send); // send data value 
}

void pedalUp(){
 byte status_byte = 176; // control change on channel 1
byte cc_number=64; // pedal control 
byte data_send = 1; // pedal UP
Serial1.print(status_byte); // send status byte
Serial1.print(cc_number); // send CC number
Serial1.print(data_send); // send data value 
}

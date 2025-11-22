#include <QTRSensors.h>
#include <Servo.h>

//motor pins
int ENA = 3, motorInput1 = 2, motorInput2 = 4;
int motorInput3 = 5, motorInput4 = 7, ENB = 6;
int BUTTON_PIN = 10;

//ir sensors
int IR[5] = {18, 17, 16, 15, 14};
int sensor_reading[5];

//speed tunning
int baseSpeedL = 70, baseSpeedR = 95;
int spinLeftL = 110, spinLeftR = -100;
int spinRightL = -100, spinRightR = 110;
int weight[5] = {-4, -2, 0, 2, 4};
int gain = 20;

//sensor calibration
QTRSensors qtr;
const uint8_t SensorCount = 5;
uint16_t sensorValues[SensorCount];

bool calibration = true;

//checkpoint variables
Servo gate;
int checkpoint = 0;
unsigned long tStart = 0;
bool fwdActive = false;
bool dropActive = false;
bool checkpointDetected = false;


//motor function
void motor(int L, int R){
  if(L>=0){digitalWrite(motorInput1,HIGH); digitalWrite(motorInput2,LOW); analogWrite(ENA,L);}
  else{digitalWrite(motorInput1,LOW); digitalWrite(motorInput2,HIGH); analogWrite(ENA,-L);}
  if(R>=0){digitalWrite(motorInput3,HIGH); digitalWrite(motorInput4,LOW); analogWrite(ENB,R);}
  else{digitalWrite(motorInput3,LOW); digitalWrite(motorInput4,HIGH); analogWrite(ENB,-R);}
}

//read sensors
void readSensors(){
  for(int i=0;i<5;i++){
    int v1=digitalRead(IR[i]); delayMicroseconds(200);
    int v2=digitalRead(IR[i]);
    sensor_reading[i] = (v1==v2? v1:0);
  }
}

//servo control
void openGate() { gate.write(180); }
void closeGate() { gate.write(120); }

//90 angle turn
void handleSharpTurn(int dir){
  Serial.println("SHARP TURN DETECTED!");

  //move forward for a short duration
  unsigned long forwardDuration = 250;
  unsigned long startTime = millis();
  while(millis() - startTime < forwardDuration){
    motor(100,125);
    readSensors();
  }
  motor(0,0); 
  delay(50);

  //spin until only center sensor sees line
  while(true){
    readSensors();
    if(dir==1) motor(spinRightL,spinRightR);
    else motor(spinLeftL,spinLeftR);
    if(sensor_reading[2]==1 && sensor_reading[0]==0 && sensor_reading[1]==0 &&
       sensor_reading[3]==0 && sensor_reading[4]==0) break;
  }

//small forward correction
  motor(-80,-55); 
  delay(30);
}

//setup
void setup(){
  Serial.begin(9600);

  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){18,17,16,15,14},SensorCount);
  delay(500);
  for(uint16_t i=0;i<400;i++) qtr.calibrate();

  for(int i=0;i<5;i++) pinMode(IR[i],INPUT);

  pinMode(ENA,OUTPUT); pinMode(ENB,OUTPUT);
  pinMode(motorInput1,OUTPUT); pinMode(motorInput2,OUTPUT);
  pinMode(motorInput3,OUTPUT); pinMode(motorInput4,OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  gate.attach(9);
  closeGate();

  Serial.println("READY");
  }


//loop
void loop(){
  byte button_pressed = digitalRead(BUTTON_PIN);

  if (calibration) {
    //show calibration status
    Serial.println("Calibrating... Press button to start");
    delay(200); // small delay to avoid spamming serial

    if (button_pressed == HIGH) { // button pressed
      calibration = false; // exit calibration mode
      Serial.println("Calibration complete! Starting robot...");
      delay(500); // small debounce
    }
    return; // skip the rest of loop until calibration done
  }

//read calibrated sensor values(0-5000) and measure line position
  uint16_t position = qtr.readLineBlack(sensorValues);

  //print sensor value from 0-1000
  //0 means max reflectance and 1000 means min reflectance
  //read calibrated values
  for(uint8_t i=0; i<SensorCount; i++){
    Serial.print(sensorValues[i]);
    Serial.print("\t");
  }

  if(checkpoint == 0){
    motor(60, 85); // move forward
  
  readSensors();

//checkpoint detection

  int sum = 0;
  for(int i=0;i<5;i++) sum += sensor_reading[i];

//lost line recovery
// if (sum == 0) {
//   Serial.println("Lost line → Forward + Spin Search");

//   motor(100, 125);
//   delay(150);

//   while (true) {
//     readSensors();
//     motor(80, -55);

//     if (sensor_reading[2] == 1) {
//       Serial.println("Line FOUND ✔");
//       break;
//     }
//   }

//   motor(0, 0);
//   delay(80);
//   return;
// }

  if(sum >= 4){
    if(!checkpointDetected){
      checkpointDetected = true;
      checkpoint++;
      tStart = millis();
      Serial.print(">>> CHECKPOINT "); Serial.println(checkpoint);

      if(checkpoint == 1 || checkpoint == 5) fwdActive = true;
      if(checkpoint == 2 || checkpoint == 3 || checkpoint == 4){
        dropActive = true;
        openGate();
      }
    }
  } else checkpointDetected = false;

//checkpoint-forward
  if(fwdActive){
    motor(120, 145);
    unsigned long duration = (checkpoint == 5 ? 800 : 400);
    if(millis() - tStart >= duration){
      fwdActive = false;
      if(checkpoint == 5){
        motor(0,0);
        while(1); // stop completely
      }
    }
    return; // priority over everything else
  }

//checkpoint-drop cube
  if(dropActive){
    motor(0,0);
    if(millis() - tStart >= 400){
      closeGate();
      dropActive = false;
    }
    return;
  }

//90 degree detection
  if((sensor_reading[2]==1 && sensor_reading[3]==1 && sensor_reading[4]==1) || 
     (sensor_reading[2]==1 && sensor_reading[3]==1) || 
     (sensor_reading[2]==1 && sensor_reading[4]==1) ||
     (sensor_reading[3]==1 && sensor_reading[4]==1)){
    handleSharpTurn(1); //right turn
    return;
  }

  if((sensor_reading[0]==1 && sensor_reading[1]==1 && sensor_reading[2]==1) || 
     (sensor_reading[0]==1 && sensor_reading[2]==1) || 
     (sensor_reading[1]==1 && sensor_reading[2]==1) ||
     (sensor_reading[0]==1 && sensor_reading[1]==1)){
    handleSharpTurn(-1); //left turn
    return;
  }

//normal line correction
  int weightedSum=0,active=0;
  for(int i=0;i<5;i++){
    if(sensor_reading[i]==1){ weightedSum+=weight[i]; active++; }
  }
  int adjustment=(active>0)? weightedSum*gain : 0;
  int L = constrain(baseSpeedL - adjustment, -100,255);
  int R = constrain(baseSpeedR + adjustment, -100,255);
  motor(L,R);
}
}

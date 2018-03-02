///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////ESC receiving side-bluetooth-W/ LED light control W/ Battery monitoring////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Servo.h>
Servo esc;

int lightpin = 3; //pin where lights FET connect
int batlevelpin = A4; //analog pin that measuers battery voltage
int btbt = 13; //bluetooth AT mode button
int escPin = 9; //esc pin out
int throttleChangeDelay = 25; //small delay after changing the esc signal
bool safe = true; //boolean that will not allow the board to run on low power
int curve = 1100; //curve variable for a variable esc signal input
int batlevel = 0; // internal battery analog reading
bool timeToSend = true; //bool to control when to send the battery status to the controller
unsigned long previousMillis = 0; //variable for a delay without using delay
const long interval = 10000; //how often to send the battery level back

void setup() {
  esc.attach(escPin); //start the esc output
  digitalWrite(btbt, LOW); //set AT mode button to ground(aka not pressed)
  pinMode(lightpin, OUTPUT); //set light FET pin to output
  digitalWrite(lightpin, LOW); //set lights to off
  Serial1.begin(9600);//start bluetooth usart
  esc.writeMicroseconds(1100); //esc setup at zero
}

void loop() {
  //start counting for our super awesome timer function
  unsigned long currentMillis = millis();
  //read battery voltage
  batlevel = analogRead(batlevelpin);
  //read serial if safe, convert data to an int, and pass it to 2 functions
  if (Serial1.available() > 0 && safe == true) {
    int throttle = normalizeThrottle( Serial1.parseInt() );
    changeThrottle(throttle);
    Lights(throttle);
  } else {
    curve = 1100;
    esc.writeMicroseconds(curve);
  }
  //delayed function for when to send the battery level to the controller. The battery status is updated every 10 seconds
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (timeToSend == true) {
      timeToSend = false;
    } else {
      timeToSend = true;
    }
  }
  //analog voltage(0-1023) to real voltage (0-5v)
  float voltage = batlevel * (5.0 / 1023.0);
  //depending on the voltage, send the apropriate command to the controller
  if (timeToSend == true) {
    if (voltage >= 4.5) {                   //Battery between 100-70%
      //Serial1.write(49);
    }
    if (voltage < 4.5 && voltage >= 3.9) {  //Battery between 70-40%
      //Serial1.write(50);
    }
    if (voltage < 3.9 && voltage >= 3.3) {  //Battery between 40-10%
      //Serial1.write(51);
    }
    if (voltage < 3.3) {                    //Battery below 10%
      //Serial1.write(52);
      //safe = false;
    }
  }
}
//function to write to the esc based on the serial and normalized with custom accelaration using the curve fucntion
void changeThrottle(int throttle) {
  if (throttle > 1100 && throttle > curve) {
    if (curve <= 1200) {
      curve += 20;
      esc.writeMicroseconds(curve);
      delay(throttleChangeDelay);
    }
    if (curve > 1200 && curve <= 1400) {
      curve += 5;
      esc.writeMicroseconds(curve);
      delay(throttleChangeDelay);
    }
    if (curve > 1400 && curve <= 1750) {
      curve += 2;
      esc.writeMicroseconds(curve);
      delay(throttleChangeDelay);
    }
  }
  if (throttle < 1100 && throttle < curve) {
    if (curve >= 1400) {
      curve -= 10;
      esc.writeMicroseconds(curve);
      delay(throttleChangeDelay);
    }
    if (curve < 1400) {
      curve -= 10;
      esc.writeMicroseconds(curve);
      delay(throttleChangeDelay);
    }
  }
  if (throttle == 1100 && curve > 1100) {
    esc.writeMicroseconds(curve);
    delay(throttleChangeDelay);
  }
  if (throttle == 1100 && curve < 1100) {
    curve = 1100;
    esc.writeMicroseconds(curve);
    delay(throttleChangeDelay);
  }
  esc.writeMicroseconds(curve);
}
//fuctnion to read the 6 or 9 from the usart and turn the lights on or off
void Lights(int throttle) {
  if (throttle == 6) {
    digitalWrite(lightpin, HIGH);
  }
  if (throttle == 9) {
    digitalWrite(lightpin, LOW);
  }
}
//function to filter the input from the usart to avoid garbage data
int normalizeThrottle(int value) {
  if ( value < 400 && value != 6 && value != 9 )
    return 400;
  if ( value > 1800 )
    return 1800;
  if ( value == 6 )
    return 6;
  if ( value == 9 )
    return 9;
  return value;
}
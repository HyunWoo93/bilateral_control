#include <math.h>


#define SAMPLE_PERIOD  (double)0.0004 // 0.4ms sample period
// Kinematics
#define HANDLE_LENGTH (double)0.011 //110.00 mm
#define SECTOR_RADIUS (double)0.082
#define MOTOR_RADIUS (double)0.0055
#define PI            (double)3.141592

void setup() {
  // put your setup code here, to run once:
  // Set up serial communication
  Serial.begin(115200);

  // Set PWM frequency
  setPwmFrequency(pwmPin, 1);

  // Input pins
  pinMode(sensorPosPin, INPUT); // set MR sensor pin to be an input

  // Output pins
  pinMode(pwmPin, OUTPUT);  // PWM pin for motor A
  pinMode(dirPin, OUTPUT);  // dir pin for motor A

  // Initialize motor
  analogWrite(pwmPin, 0);     // set to not be spinning (0/255)
  digitalWrite(dirPin, LOW);  // set direction


  //initialize output compare
  InitOC();
}

// pin variables
int sensorPosPin = A2; // input pin for MR sensor
int pwmPin = 5; // PWM output pin for motor 1
int dirPin = 8; // direction output pin for motor 1

// variables for position calculation
int updatedPos = 0;     // keeps track of the latest updated value of the MR sensor reading
int prev_updatedPos = 0;// for velocity calc
int rawPos;
int lastRawPos = 0;     // last raw reading from MR sensor
int lastLastRawPos = 0; // last last raw reading from MR sensor
int flipNumber = 0;     // keeps track of the number of flips over the 180deg mark
int tempOffset = 0;
int rawDiff = 0;
int lastRawDiff = 0;
int rawOffset = 0;
int lastRawOffset = 0;
const int flipThresh = 900;  // threshold to determine whether or not a flip over the 180 degree mark occurred
boolean flipped = false;

// for bias
float thetaBias = 0;
bool bSetBias = true;

// for velocity calculation
float theta, omega, omega_uf;
float prev_theta;
float theta_diff, prev_theta_diff;
float prev_omega, prev_omega_uf;

// target
float theta_t = 0;
float omega_t = 0;

// Force output variables
double Th = 0;           // force at the handle
double Tp = 0;              // torque of the motor pulley
double duty = 0;            // duty cylce (between 0 and 255)
unsigned int output = 0;    // output command to the motor
double K = 0.0;
double B = 0.0;

// porotocol
byte packet[3] = {NULL}
byte theta_self[2] = {NULL}
int theta_temp = 0.0;
bool get_header = false;


void loop() {

  // init
  int i;
  for(i = 0; i < 3; i++){
    packet[i] = NULL;
  }

  // load header
  packet[0] = 'T';


  // load data
  theta_temp = (theta + 90) * PI * 10000/ 180.0
  theta_self[0] = (byte)(theta_temp / 256);
  theta_self[1] = (byte)(theta_temp % 256);

  // send packet
  if(Serial.available() == 0){
    int i;
    for(i = 0; i < 3; i++){
      Serial.write(packet[i]);
    }
  }


  //Serial.write('P');

  //Serial.println(theta);

  
  //*************************************************************
  //*** Section 3. Assign a motor output force in Newtons *******
  //*************************************************************
  /*
  Th = -K_s * (theta - theta_t) - B_s * (omega - omega_t);
  if (theta > theta_t)
    Th = - B_s * (omega - omega_t);

  Tp = (MOTOR_RADIUS / SECTOR_RADIUS) * Th;

  // Determine correct direction for motor torque
  if (Th < 0)
  {
       digitalWrite(dirPin, HIGH);
  }
  else
  {
       digitalWrite(dirPin, LOW);
  }

  // Compute the duty cycle required to generate Tp (torque at the motor pulley)c
  duty = sqrt(abs(Tp) / 0.03);

  // Make sure the duty cycle is between 0 and 100%
  if (duty > 1)
  {
    duty = 1;
  }
  else if (duty < 0)
  {
    duty = 0;
  }
  output = (int)(duty * 255);  // convert duty cycle to output signal
  analogWrite(pwmPin, output); // output the signal
  */
}


int dataMode = -1;  // 0: stiffness, 1: damping, 2: target
int dataCnt = -1;
bool bWaitEnd = false;
byte targetByte[2];

void serialEvent()
{
  while (Serial.available())
  {
    byte b = Serial.read();
    
    // header?
    char c = (char)b;
    if (c == 'K' || 'C'){
      get_header = true;

    }
    else if(c == 'S' || 'P' || 'B'){
      get_header = true;

    }
    else if(c == 'T'){
      get_header = true;
    }

    // data
    else{
      if (b == 0x00){

      }
      
    }

  }
}

//calculated "updatedPos" variable

void calculatePosition()
{
  //toggle bit 5 high (pin 13)
  //PORTB |= B00100000;

  //update "previous updatedPos" before updating position
  prev_updatedPos = updatedPos;

  // Get voltage output by MR sensor
  rawPos = analogRead(sensorPosPin);  //current raw position from MR sensor

  // Calculate differences between subsequent MR sensor readings
  rawDiff = rawPos - lastRawPos;          //difference btwn current raw position and last raw position
  lastRawDiff = rawPos - lastLastRawPos;  //difference btwn current raw position and last last raw position
  rawOffset = abs(rawDiff);
  lastRawOffset = abs(lastRawDiff);

  // Update position record-keeping vairables
  lastLastRawPos = lastRawPos;
  lastRawPos = rawPos;

  // Keep track of flips over 180 degrees
  if ((lastRawOffset > flipThresh) && (!flipped))
  {
    // enter this anytime the last offset is greater than the flip threshold AND it has not just flipped
    if (lastRawDiff > 0)
    {
      // check to see which direction the drive wheel was turning
      flipNumber--;              // cw rotation
    }
    else
    {
      // if(rawDiff < 0)
      flipNumber++;              // ccw rotation
    }
    if (rawOffset > flipThresh)
    {
      // check to see if the data was good and the most current offset is above the threshold
      updatedPos = rawPos + flipNumber * rawOffset; // update the pos value to account for flips over 180deg using the most current offset
      tempOffset = rawOffset;
    }
    else
    {
      // in this case there was a blip in the data and we want to use lastactualOffset instead
      updatedPos = rawPos + flipNumber * lastRawOffset; // update the pos value to account for any flips over 180deg using the LAST offset
      tempOffset = lastRawOffset;
    }
    flipped = true;            // set boolean so that the next time through the loop won't trigger a flip
  }
  else
  {
    // anytime no flip has occurred
    updatedPos = rawPos + flipNumber * tempOffset; // need to update pos based on what most recent offset is
    flipped = false;
  }


}

//function for calculating actual position and velocity in m, m/s
void getVelocity()
{
  static double prev_xh;
  static double vel_h_uf;  //unfilitered velocity
  static double prev_vel_h_uf; //previous unfiltered velocity
  static double prev_vel_h; //previous filtered velocity
  //*************************************************************
  //*** Section 2. Compute position in meters *******************
  //*************************************************************

  // Step B.6: double ts = ?; // Compute the angle of the sector pulley (ts) in degrees based on updatedPos
  //y = -0.00970857x + 7.33554011

  theta = (-0.0097086 * 1.5 * updatedPos + 7.33554) * PI / 180 - thetaBias; //in radian
  prev_theta = (-0.0097086 * 1.5 * prev_updatedPos + 7.33554) * PI / 180 - thetaBias; //in radian

  prev_omega_uf = omega;

  prev_theta_diff = theta_diff;
  theta_diff = theta - prev_theta;
  theta_diff = (double)((int)(theta_diff * 1000)) / 1000.0;

  if (abs(theta_diff) > 0.01)
    theta_diff = prev_theta_diff;
  omega_uf = theta_diff / SAMPLE_PERIOD;

  if (bSetBias)
  {
    thetaBias = theta + thetaBias;
    bSetBias = false;
  }

  prev_omega = omega;
  omega = 0.4399 * omega_uf + 0.4399 * prev_omega_uf + 0.1202 * prev_omega; //discrete low pass filter
  //  vh = -(.95 * .95) * lastLastVh + 2 * .95 * lastVh + (1 - .95) * (1 - .95) * (xh - lastXh) / .0001; // filtered velocity (2nd-order filter)
}

// --------------------------------------------------------------
// Function to set PWM Freq -- DO NOT EDIT
// --------------------------------------------------------------
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if (pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch (divisor)
    {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if (pin == 5 || pin == 6)
    {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    }
    else
    {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  }
  else if (pin == 3 || pin == 11)
  {
    switch (divisor)
    {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}

//initializes the output compare on timer 1 for performing calculations
void InitOC()
{
  //*******OUTPUT COMPARE ON CHANNEL 1******

  //see section 15.7: of ATmega328 datasheet
  //register descriptions section 17.11, page 158

  //  DDRB |= _BV(1); //set bit 5 as output direction, OC1A debug pin (Pin 9) ---> PIN 9 IS DIRECTION
  //B5 (11) will oscillate at half the freq

  //WGM3:0 set to 0,1,0,0 for output compare (clear timer on compare)
  //CTC mode, TOP = OCR1A, update immediate, TOV flag set on MAX
  //from table 15-4
  TCCR1A &= ~_BV(WGM11) & ~_BV(WGM10);
  TCCR1B &= ~_BV(WGM13);
  TCCR1B |= _BV(WGM12);

  //debug output
  //COM1A1:0 set to 0,1 for toggle --> set OC1A (B1) bit high (will clear at end of calculation) --> just toggle instead
  //from table 15-1 on page 134
  TCCR1A |= _BV(COM1A1) | _BV(COM1A0);
  TCCR1A &= ~_BV(COM1A1);

  //CS12:0 set to 1,0,0 for clk/256 prescaling: runs on 16MHz clock
  //table 15-5
  TCCR1B &= ~_BV(CS10) & ~_BV(CS11);
  TCCR1B |= _BV(CS12);

  //****CHANGE OCR1A TO MODULATE FREQUENCY
  //page 126: freq = f_clk / (N * (1 + OCR1A)), N = 256, f_clk = 16MHz (the 2 in the equation is because of toggling)
  //want period of 400us --> 2.5kHz
  //2.5kHz = 16 MHz / (256 * (1 + OCR1A)) --> OCR1A = 24
  OCR1A = 24;

  //interrupt enable

  TIMSK1 |= _BV(OCIE1A); //output compare interrupt timer 1A

}

//Timer 1 interrupt for performing position calculations
ISR(TIMER1_COMPA_vect)
{
  PORTB |= B00100000;
  //*************************************************************
  //*** Section 1. Compute position in counts (do not change) ***
  //*************************************************************

  calculatePosition();
  getVelocity();
  PORTB &= ~B00100000;
  //PORTB &= ~B00000001; //clear bit B1
}
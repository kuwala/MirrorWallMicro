/* Servo
Miuzei 10 Pcs Sg90 9g Micro Servo Metal Gear servo motor kit

180 Degree Servo Motor Control Specification：

    Control system:Change the pulse width
    Amplifier type:Digital controller
    Operating travl:180° (500-2500 μsec)
    Left&Right travelling Angle deviation:≤ 6°
    Centering deviation:≤ 1°
    Neutral position:1500 μsec
    Dead band width:5 μsec
    Rotating direction:Counter Clockwise (500-2500μsec)
    Pulse width range:500-2500μsec
    Maximum travel:About 180° (500-2500 μsec)

    product:
    https://www.amazon.com/Micro-Helicopter-Airplane-Remote-Control/dp/B072V529YD
    */
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
Adafruit_PWMServoDriver pwm2 = Adafruit_PWMServoDriver(0x41);
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);
// you can also call it with a different address and I2C interface
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40, Wire);

// Depending on your servo make, the pulse width min and max may vary, you 
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!
// #define SERVOMIN  150 // This is the 'minimum' pulse length count (out of 4096)
// #define SERVOMAX  580 // This is the 'maximum' pulse length count (out of 4096)
// #define SERVOMIN  10 // This is the 'minimum' pulse length count (out of 4096)
// #define SERVOMAX  580 // This is the 'maximum' pulse length count (out of 4096)
#define SERVOMIN  110 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  500 // This is the 'maximum' pulse length count (out of 4096)
#define USMIN  600 // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX  2400 // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600

#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

int const rows = 4;
int const cols = 8;
uint16_t servoValues[rows][cols]; // current values to set this update loop
uint16_t servoLast[rows][cols];
uint16_t servoTargets[rows][cols];

byte servoBytes[rows][cols]; // read from serial
int const stepMin = 30; // if the servoTarget - servoValue is less then stepMin servoValue = servo Target

unsigned long stepTimer = 0;
int stepCount = 0;
unsigned long pwmUpdateTimer = 0;

void setServoTargetTo(uint16_t servoX, uint16_t servoY, uint16_t value) {
  if (value > SERVOMAX) {
    value = SERVOMAX;
  } else if (value < SERVOMIN) {
    value = SERVOMIN;
  }
  servoTargets[servoY][servoX] = value;
}
void setServoTo(uint16_t servoX, uint16_t servoY, uint16_t value) {
  if (value > SERVOMAX) {
    value = SERVOMAX;
  } else if (value < SERVOMIN) {
    value = SERVOMIN;
  }
  servoValues[servoY][servoX] = value;
}
void setAllServoGridTo(uint16_t value) {
  if (value > SERVOMAX) {
    value = SERVOMAX;
  } else if (value < SERVOMIN) {
    value = SERVOMIN;
  }
  for(int i = 0; i < rows; i ++) {
    for(int j = 0; j < cols; j++) {
      // pwm.setPWM(j+(i*j), 0, value);
      servoValues[i][j] = value;
    }
  }
}

void calculateNewServoValuesFromTargets() {
  for(int i = 0; i < rows; i ++) {
    for(int j = 0; j < cols; j++) {
      servoLast[i][j] = servoValues[i][j];
      int servoDistance = servoTargets[i][j] - servoValues[i][j];
      if (abs(servoDistance) > stepMin) {
        servoValues[i][j] += servoDistance / 2;
      } else {
        servoValues[i][j] = servoTargets[i][j];
      }
    }
  }
}

void updateServos() {
  for(int i = 0; i < rows; i ++) {
    for(int j = 0; j < cols; j++) {
      // only update if different from last update
      if(servoLast[i][j] != servoValues[i][j]) {
        switch(i) {
          case 0:
            pwm.setPWM(j, 0, servoValues[i][j]);
            break;
          case 1:
            pwm.setPWM(8+j, 0, servoValues[i][j]);
            break;
          case 2:
            pwm2.setPWM(j, 0, servoValues[i][j]);
            break;
          case 3:
            pwm2.setPWM(8+j, 0, servoValues[i][j]);
            break;
          default:
          Serial.print("strange pwm index - ignoring update");
        }
      }
    }
  }
}

// dont use
void updateServosFromGrid() {
  for(int i = 0; i < rows; i ++) {
    for(int j = 0; j < cols; j++) {
      
      Serial.print("-");
      if(i == 0) {
        Serial.print("S.");
        pwm.setPWM(j, 0, servoValues[i][j]);
      } else if (i == 1) {
        Serial.print("S2.");
        pwm2.setPWM(j, 0, servoValues[i][j]);
      }

    }
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println("8 channel Servo test!");

  pwm.begin();
  pwm2.begin();
  /*
   * In theory the internal oscillator (clock) is 25MHz but it really isn't
   * that precise. You can 'calibrate' this by tweaking this number until
   * you get the PWM update frequency you're expecting!
   * The int.osc. for the PCA9685 chip is a range between about 23-27MHz and
   * is used for calculating things like writeMicroseconds()
   * Analog servos run at ~50 Hz updates, It is importaint to use an
   * oscilloscope in setting the int.osc frequency for the I2C PCA9685 chip.
   * 1) Attach the oscilloscope to one of the PWM signal pins and ground on
   *    the I2C PCA9685 chip you are setting the value for.
   * 2) Adjust setOscillatorFrequency() until the PWM update frequency is the
   *    expected value (50Hz for most ESCs)
   * Setting the value here is specific to each individual I2C PCA9685 chip and
   * affects the calculations for the PWM update frequency. 
   * Failure to correctly set the int.osc value will cause unexpected PWM results
   */
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates
  pwm2.setOscillatorFrequency(27000000);
  pwm2.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates

  delay(10);
}

// You can use this function if you'd like to set the pulse length in seconds
// e.g. setServoPulse(0, 0.001) is a ~1 millisecond pulse width. It's not precise!
double setServoPulse(uint8_t n, double pulse) {
  double pulselength;
  
  pulselength = 1000000;   // 1,000,000 us per second
  pulselength /= SERVO_FREQ;   // Analog servos run at ~60 Hz updates
  Serial.print(pulselength); Serial.println(" us per period"); 
  pulselength /= 4096;  // 12 bits of resolution
  Serial.print(pulselength); Serial.println(" us per bit"); 
  pulse *= 1000000;  // convert input seconds to us
  pulse /= pulselength;
  Serial.println(pulse);
  return pulse;
  // pwm.setPWM(n, 0, pulse);
}

void loop() {

  // if (millis() - stepTimer > 4000) {
  //   if (stepCount == 0) {
  //     setServoTargetTo(0,0, SERVOMAX);
  //     // setServoTargetTo(4+random(4),0, random(SERVOMAX));
  //     stepCount++;
  //   } else if (stepCount == 1) {
  //     setServoTargetTo(0,0, SERVOMIN);
  //     // setServoTargetTo(4+random(4),0 , random(SERVOMAX));
  //     stepCount++;
  //   } else if (stepCount == 2) {
  //     setServoTargetTo(1,0, SERVOMAX);
  //     // setServoTargetTo(4+random(4),0 , random(SERVOMAX));
  //     stepCount++;
  //   } else if (stepCount == 3) {
  //     setServoTargetTo(1,0, SERVOMAX);
  //     // setServoTargetTo(4+random(4),0, random(SERVOMAX));
  //     stepCount = 0;
  //   }
  //   stepTimer = millis();
  //   Serial.println("time step happened");
  // } 
  //get serial values
  if(Serial.available()>= rows * cols) {
    for(int i = 0; i < rows * cols; i ++) {
      byte b = Serial.read();
      int x = i % cols;
      int y = i / rows;
      servoBytes[x][y] = b;
      Serial.print("byte: ");
      Serial.println((int)b);
      if(b == 49) { // 1
        setServoTargetTo(x,y, SERVOMIN);
      } else if (b==50) { // 2
        setServoTargetTo(x,y, SERVOMAX);
      }
    }
  }
  if (millis() - pwmUpdateTimer > 10) {
    calculateNewServoValuesFromTargets();
    pwmUpdateTimer = millis();
  }

  updateServos();

  // setServoPulse(0,0.0005); //0.5miliseconds
  // setServoPulse(0,0.0025); // 2.5ms
  // POWER TEST CODE
  // Serial.println("starting loop");
  // setAllServoGridTo(SERVOMIN);
  // updateServosFromGrid();
  // delay(400);
  // Serial.println("mid loop");
  // setAllServoGridTo(SERVOMAX);
  // updateServosFromGrid();
  // delay(400);

  // Code for sprinkler effect
  /*
  for(int i = 0; i < 40; i++) {
    setAllServoGridTo(SERVOMIN+(i*10));
    // setServoTo(0, (i+3)%16, SERVOMAX-(i*3));
    updateServosFromGrid();
    delay(i*20);
  }
  for(int i = 40; i >0; i--) {
    setAllServoGridTo(SERVOMIN+(i*10));
    // setServoTo(0, i%16, SERVOMAX-(i*3));
    updateServosFromGrid();
    delay(i*2);
  } */

  // for(int i = 0; i < rows; i ++) {
  //   setServoTo(i, 0, SERVOMIN+(i*10));
  //   updateServosFromGrid();
  //   delay(i*7);
  // }
  // for(int i = 0; i < rows; i ++) {
  //   setServoTo(i, 1, SERVOMIN+(i*30));
  //   updateServosFromGrid();
  //   delay(i*9);
  // }
  // delay(4000);
}
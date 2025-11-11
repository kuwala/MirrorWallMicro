/* TowerPro SG92R Servos - pulse calculations
the period is 20ms, high level time 0.5~2.5ms 
50hz = 20,000uS
12bit = 2^12 = 4096 increments
20,000 / 4096 = 4.88uS per increment

minimum 500us / 4.88uS = 102.44
maximum 2500us / 4.88uS = 512.29
// from https://forums.adafruit.com/viewtopic.php?t=96423
*/
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
// Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
// Adafruit_PWMServoDriver pwm2 = Adafruit_PWMServoDriver(0x41);
int const numPWMBoards = 8;
Adafruit_PWMServoDriver pwmBoards[numPWMBoards];

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

#define SERVOMIN  100 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  500 // This is the 'maximum' pulse length count (out of 4096)
#define SERVONINTEY 300// this is aprox 90 degrees from

// #define SERVOMIN  130 // This is the 'minimum' pulse length count (out of 4096)
// #define SERVOMAX  480 // This is the 'maximum' pulse length count (out of 4096)
#define USMIN  600 // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX  2400 // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600

#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

int const rows = 4;
int const cols = 8;
int const rowsPerBoard = 2;
int const colsPerBoard = 8;
uint16_t servoValues[rows][cols]; // current values to set this update loop
uint16_t servoLast[rows][cols];
uint16_t servoTargets[rows][cols];
uint16_t servoSerialBuffer[rows][cols]; // stored Serial data before its complete and pushed
unsigned long servosTimers[rows][cols]; // used for debouncing input with noise camera pixels
uint16_t pixelDebounce = 700; // milliseconds
bool enabledDebounce = false;

byte servoBytes[rows][cols]; // read from serial

// with boardMap you can convert a x,y servo positon
// to pwmBoardNumber. 
// first boardX = floor(x/colsPerBoard);
// then boardY = floor(y/rowsPerBoard);
// uint8_t boardMap[12][3] = {
//   {0, 4, 8},
//   {1, 5, 9},
//   {2, 6, 10},
//   {3, 7, 11},
//   {12, 16, 20},
//   {13, 17, 21},
//   {14, 18, 22},
//   {15, 19, 23},
//   {24, 28, 32},
//   {25, 29, 33},
//   {26, 30, 34},
//   {27, 31, 35},
// };
uint8_t boardMap[12][3] = {
  {0, 4, 8},
  {1, 5, 9},
  {2, 6, 10},
  {3, 7, 11},
  {20, 16, 12},
  {21, 17, 13},
  {22, 18, 14},
  {23, 19, 15},
  {32, 28, 24},
  {33, 29, 25},
  {34, 30, 26},
  {35, 31, 27},
};

uint8_t boardNumToYOffset[36] = {
  0,2,4,6,     0,2,4,6,     0,2,4,6,
  8,10,12,14,  8,10,12,14,  8,10,12,14,
  16,18,20,22, 16,18,20,22, 16,18,20,22
};
uint8_t boardNumToXOffset[36] = {
  0,0,0,0, 8,8,8,8, 16,16,16,16,
  0,0,0,0, 8,8,8,8, 16,16,16,16,
  0,0,0,0, 8,8,8,8, 16,16,16,16
};

// servo update step
int const stepMin = 30; // if the servoTarget - servoValue is less then stepMin: servoValue = servo Target

unsigned long stepTimer = 0;
int stepCount = 0;
unsigned long pwmUpdateTimer = 0;

uint8_t bigSerialBufferRx [rows*cols+16];

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
void noSmoothCalculateNewServoValuesFromTargets() {
  for(int i = 0; i < rows; i ++) {
    for(int j = 0; j < cols; j++) {
      //servoLast[i][j] = servoValues[i][j];
        servoValues[i][j] = servoTargets[i][j];
    }
  }
}

void updateServos() {
  for(int i = 0; i < rows; i ++) {
    for(int j = 0; j < cols; j++) {
      // only update if different from last update
      if(servoLast[i][j] != servoValues[i][j] && ((millis() - servosTimers[i][j]) > pixelDebounce)) {
            servosTimers[i][j] = millis();
            // Serial.print(".u"); // update
            uint8_t boardNum = boardMap[(int)(i/rowsPerBoard)][(int)(j/colsPerBoard)];
            uint8_t servoNum = j%colsPerBoard + (i%rowsPerBoard)*colsPerBoard;
            // Serial.print("pushing changed pwm to servo. Y: ");
            // Serial.print(i);
            // Serial.print(" X: ");
            // Serial.print(j);
            // Serial.print(" BoardNum: "); Serial.println(boardNum);
            pwmBoards[boardNum].setPWM(servoNum, 0, servoValues[i][j]);
            servoLast[i][j] = servoValues[i][j];
      } else {
        //ignoring same as lastUpdate Servo value
      }
      // Serial.println(".e");// end
    }
  }
}

// dont use
// void updateServosFromGrid() {
//   for(int i = 0; i < rows; i ++) {
//     for(int j = 0; j < cols; j++) {
      
//       Serial.print("-");
//       if(i == 0) {
//         Serial.print("S.");
//         pwm.setPWM(j, 0, servoValues[i][j]);
//       } else if (i == 1) {
//         Serial.print("S2.");
//         pwm.setPWM(j+8, 0, servoValues[i][j]);
//       } else if (i == 2) {
//         pwm2.setPWM(j, 0, servoValues[i][j]);
//       } else if (i == 3) {
//         pwm2.setPWM(j+8, 0, servoValues[i][j]);
//       }

//     }
//   }
// }


void setup() {
  //init the pwm boards
  for(int i=0;i<numPWMBoards;i++) {
    pwmBoards[i] = Adafruit_PWMServoDriver(0x40+i);
    pwmBoards[i].begin();
    pwmBoards[i].setOscillatorFrequency(27000000);
    pwmBoards[i].setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates
  }
  Serial.begin(115200);
  // I think this  would work if I had a hardware serial connection. But the regular Serial is only for USB serial
  // HardwareSerialIMXRT.addMemoryForRead(&bigSerialBufferRx, sizeof(bigSerialBufferRx));
  // Serial1.addMemoryForRead(&bigSerialBufferRx, sizeof(bigSerialBufferRx));
  Serial.println("MirroWall Servo Controller Started");

  // pwm.begin();
  // pwm2.begin();

  // pwm.setOscillatorFrequency(27000000);
  // pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates
  // pwm2.setOscillatorFrequency(27000000);
  // pwm2.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates
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

  delay(10);
}

// You can use this function if you'd like to set the pulse length in seconds
// e.g. setServoPulse(0, 0.001) is a ~1 millisecond pulse width. It's not precise!
// setServoPulse is not used
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
  bool testing = false;
  if (millis() - stepTimer > 100 && testing) {
    if (stepCount == 0) {
      // setServoTargetTo(2,0, SERVOMAX);
      for(int i = 0; i < rows; i ++) {
        for(int j = 0; j < cols; j++ ) {
          setServoTargetTo(j,i, SERVOMIN);
        }
      }
      // setServoTargetTo(4+randm(4),0, random(SERVOMAX));
      stepCount++;
    } else if (stepCount == 1) {
      // setServoTargetTo(2,0, SERVOMIN);
      for(int i = 0; i < rows; i ++) {
        for(int j = 0; j < cols; j++ ) {
          setServoTargetTo(j,i, SERVONINTEY);
        }
      }
      // setServoTargetTo(4+random(4),0 , random(SERVOMAX));
      stepCount++;
      stepCount = 0;
    } else if (stepCount == 2) {
      setServoTargetTo(1,0, SERVOMAX);
      // setServoTargetTo(4+random(4),0 , random(SERVOMAX));
      stepCount++;
    } else if (stepCount == 3) {
      setServoTargetTo(1,0, SERVOMIN);
      // setServoTargetTo(4+random(4),0, random(SERVOMAX));
      stepCount = 0;
    }
    stepTimer = millis();
    Serial.println("time step happened");
  } 
  // Do the special Serial!
  if(Serial.available()>= 17) { // 17 bytes per data packet
    // Serial.println("serial bytes received, reading starting");
    uint8_t header = Serial.read();
    Serial.print("Header: ");
    Serial.println(header);
    uint16_t pwmBoardNum = 0;
    if(header <= numPWMBoards) {
      pwmBoardNum = header;
      for(int i = 0; i < 16; i ++) {
        byte b = Serial.read();
        int x = i % colsPerBoard + boardNumToXOffset[pwmBoardNum]; // colsPerBoard was 8 before
        int y = floor(i / colsPerBoard) + boardNumToYOffset[pwmBoardNum];
        // servoBytes[y][x] = b; // ??
        // Serial.print("byte: ");
        // Serial.print((int)b);
        // Serial.print(" x: ");
        // Serial.print(x);
        // Serial.print(" y: ");
        // Serial.print(y);

        uint16_t value;
        if(b == 49) { // 1
          value = SERVOMIN;
        } else if (b==50) { // 2
          value = SERVOMAX;
        } else if (b==51) { // 3
          value = SERVONINTEY;
        } else {
          value = SERVOMIN;
          Serial.println(" * * * * * * * * * * * * * * * * * * * * * * * * ");
          Serial.println(" * * * unknown value received via serial * * * * ");
          Serial.println(" * * * * * * * * * * * * * * * * * * * * * * * * ");
        }
        // Serial.print(" value: ");
        // Serial.println(value);

        // servoTargets[y][x] = value;
        servoSerialBuffer[y][x] = value;

      }
    } else if(header == 255) {
      // set receivedSerial to revoTarget
      // discard next 16 bytes
      for(int i = 0; i < 16; i ++) {
        Serial.read();
      }
      Serial.println("Drawing Buffered Serial Data");
      for(int y = 0; y < rows; y ++) {
        for(int x = 0; x < cols; x++) {
          servoTargets[y][x] = servoSerialBuffer[y][x];
          // Serial.print(servoSerialBuffer[y][x]);
          // Serial.print(".x");
        }
        // Serial.println("|");
      }
      // Serial.println("done.");

    } else {
      Serial.println("* * * * Invalid Header Byte Received * * * * ");
      Serial.println("* * * * Invalid Header Byte Received * * * * ");
      Serial.println("* * * * Invalid Header Byte Received * * * * ");
    }

  }

  //get serial values
  // this old method only works up to 64 bytes. the size of the buffer
  // if(Serial.available()>= rows * cols) {
  //   Serial.println("serial received, reading starting");
  //   for(int i = 0; i < rows * cols; i ++) {
  //     byte b = Serial.read();
  //     int x = i % cols;
  //     int y = i / cols;
  //     // servoBytes[y][x] = b; // ??
  //     Serial.print("byte: ");
  //     Serial.print((int)b);
  //     Serial.print("x: ");
  //     Serial.print(x);
  //     Serial.print("y: ");
  //     Serial.println(y);
  //     if(b == 49) { // 1
  //       setServoTargetTo(x,y, SERVOMIN);
  //     } else if (b==50) { // 2
  //       setServoTargetTo(x,y, SERVOMAX);
  //     }
  //   }
  // }

  if (millis() - pwmUpdateTimer > 10) {
    // calculateNewServoValuesFromTargets();
    noSmoothCalculateNewServoValuesFromTargets();
    pwmUpdateTimer = millis();
  }

  updateServos();

  // setServoPulse(0,0.0005); //0.5miliseconds
  // setServoPulse(0,0.0025); // 2.5ms
  // POWER TEST CODE
  // Serial.println("starting loop");
  // setAllServoGridTo(SERVOMIN);
  // updateServosFromGrid();
  // delay(1200);
  // Serial.println("mid loop");
  // setAllServoGridTo(SERVOMAX);
  // updateServosFromGrid();
  // delay(1200);

  // POWER TEST TWO
  // for(int i = 0; i < 16; i++) {
  //   pwm.setPWM(i, 0, SERVOMIN);
  //   delay(200);
  // }
  // delay(2000);
  // for(int i = 0; i < 16; i++) {
  //   pwm.setPWM(i, 0, SERVOMAX);
  //   delay(200);
  // }
  // delay(1000);

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
# MirrorWall Project
Tried to get it going with a teensy as the main micro controller but it was not working, so I switched to an arduino. Will try again with different servos. Maybe the servos we used dont support the 3.3v logic levels? but I assumed that the pwm module PCA9685 would boost the pwm voltage to 5v. I should check if the PWM voltage is actually being sent to the PCA9685 board.

Servo Details are:
```
SERVO
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
```

Some microsecond calculations
for 50hz servos
```bash
20000.00 us per period
4.88 us per bit
102400.00
20000.00 us per period
4.88 us per bit
512000.00
```

Product Name: SG92R
Product size: 22.4*12.5*23.8mm
Product weight: 10 grams of earth 5%
Product line length: 25cm soil 1cm
Working voltage: 4.8V-6V
Blocking torque: 1.3KG.CM-1.8KG.CM
No-load speed: 0.09 sec/60
No-load current: 90MA
Output shaft: 20t

Product Description:Highlight：
High-quality copper wire
High quality nylon gears
High-speed strong magnetic motor

Kindly Reminder：
- Please use the standard pulse width modulation control signal, the period is 20ms, high level time 0.5~2.5ms
- 270 degrees and 180 degrees is the same, can stop at any angle, is adjustable angle
- 360 degrees is a continuous rotation, can not control the angle and speed
- 180 degrees and 270 degrees can control the angle, PWM 500-2500
- Can be remotely controlled 180 ° and 270 °
- If you need remote control, please contact customer service first

- Please use the standard pulse width modulation control signal, the period is 20ms, high level time 0.5~2.5ms 

sg92r servos
14 servos
2 pwm boards
1 arduino
1 teensy 4
peeked around 3.4amps

with 3 removed 3

1.3 amps per 7 servos

Okay so the update so far. Each servo uses about 10ma at idle and arond 100-300ma when moving. (if its changing direction of motion rapidly is 300ish). 

The servos have around 180 degrees. from my tests. The origin point on each one maybe a few degrees off but its close.

- [ ] 5.5 hours total so far. (or more..?)

## 09/28/25 Sunday
The power meters only work when they receive power via their extra power header. At least in our case with 5v power. From youtube
videos they seem to work fine without the extra power on that.
16 servos take about 3.2 Amps max when all moving at the same time. 
576 servos / 16 (servos) = 36 sets of * 3.2AMps
36 * 3.2 = 115A for a 3x3 ft section

So if we want to do 8x8 servos. I should probably prepare 
8 x 4
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0

### todo code wise
- target, last, current values
- make pwm array
- map [][] to pwm array 

when the loop starts
move current servo values to last servo values
calculate new current values based on target values
update servos

// what are some options for deserializing data bytes
receive serial bytes 0-180 in value and map those to servo positions

## 09/28/25 Monday
- update code to work with all 32 servos
- prep for connecting to the 32 servo board


What couldbe the problem to getting the serial after 4 bytes ignored .... hmm 

## 09/30/25

## 10/01/25
calibrateing the servo pwm driver pulses
https://learn.adafruit.com/calibrating-sensors/two-point-calibration
https://forums.adafruit.com/viewtopic.php?t=96423
* there maybe up to a 5% difference between the individual pwm driver boards

- trouble shoot encodings
- test to get a bunch more servos going.
- maybe trouble shoot the a frame buzzing or humming

So the problem seems to be that only a few of the servos are being set correctly.
it could be an issue with the x and y being mixed up or my switch case.

So what do we need to prepare to make the bigger version.

```bash
Semiconductors, especially the relatively delicate pins of digital ICs, cannot sink an infinite amount of current. Please take a look at the official I2C specification from NXP. 3 mA for Standard-mode and Fast-mode, or 20 mA for Fast-mode Plus. 

https://www.nxp.com/docs/en/user-guide/UM10204.pdf

For achieving longer distance without using buffers, it is possible to use series termination resistors to slow down the edge rate and reduce ringing. The I2C specification calls them "series protection resistors". I had success with using 330 ohms for series and 3.3k for pull-ups for a 3.3V I2C bus less than 5 feet while running at a relatively slow speed of 10 kHz. Your mileage will vary depending on the type of cable used.

I2C is designed for communication between ICs on a single board or boards stacked on top of each other, not for boards connected by long wires. For long distances, you'll need some sort of buffer or redriver.
```

## 10/11/25
All the parts arrived a few days ago and I picked up some female headers
Some Tasks
- [ ] mount all the motors
- [ ] solder a test batch of pwm boards (boards + extention wire    x)

## (sometime) work log
- went to store to buy supplies (45 mins?)
- mounted ~30 + motors (3hrs) (my estimate its going to take 20 hours to install all the motors)
- built frame (2-3 hrs)

1hr
- soldered 4 boards 
- made 3 i2c cables, soldered to boards
## 10/18/25
- build the arduino shield (?)
- solder stemma cable to shield (or wait, just use 2 sets of 2 header pins)
- daisy chain from i2c extender to the 4 pwm boards
- test and reset the motors
- put on horns on the motors
- install many more motors

- work on code to play an animation of png files and send serial data


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

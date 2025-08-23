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



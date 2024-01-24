# Control 2 DRV8825 step motor driver with timer on Arduino Uno R3
This is my project with another one student to control 2 step motor 
## Requirement
- 2 DRV8825
- LCD with I2C
- Arduino Uno R3
- 2 Bipolar Step motor
## Features
- Generate pulse without using delay on Arduino Uno R3
## Tech
- This project using 2 Timer 1 and Timer 2 to generate square wave that control 2 driver of 2 step motor
- The square wave caculation like this below image
![alt text](https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcTxJQd-1ZBy00zNcst6owdo6XkX8FRd9f9rrg&usqp=CAU)
- How to generate square wave using timer please refer to ATmega datasheet and my comment inside ino file: [Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/doc2503.pdf)


## Installation
You need to install LiquidCrystal_I2C library for LCD

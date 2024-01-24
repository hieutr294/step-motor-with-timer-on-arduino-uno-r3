#include <LiquidCrystal_I2C.h>

volatile unsigned long top = 0;
volatile unsigned long top2 = 0;

// datashet https://ww1.microchip.com/downloads/en/DeviceDoc/doc2503.pdf

LiquidCrystal_I2C lcd(0x27,20,4);

byte set = 3;
byte up = 2;
byte down = 4;

byte enaX = 5;
byte dirX = 6;

byte enaY = 8;
byte dirY = 10;

int prevMenuState = 0;

int xValue = 0;
int yValue = 0;

int prevXvalue = 0;
int prevYvalue = 0;

volatile long distanceX = 0;
volatile long distanceY = 0;

int menuState = 0;
char* currentMenu = "";
char* upDownFunc = "normal";

void timer1Init() {
  TCCR1A = 0; TCCR1B = 0; TIMSK1 = 0; // reset timer 1 (datasheet ATmega32 page 109)

  DDRB |= (1 << PB1); // configure PB1 (pin 9) as output (datasheet ATmega32 page 109)

  TCCR1A |= (1 << WGM11) | (1 << COM1A1) | (1 << COM1B1);  // configure COM1A1 and COM1B1 = 1 for non-inverting mode (datasheet ATmega32 page 109)

  TCCR1B |= (1 << WGM12) | (1 << WGM13); // WGM11 WGM12 WGM13 = 1 for fast PWM mode (datasheet ATmega32 page 109)

  TIMSK1 |= (1 << OCIE1A); // Enable interrupt when TCNT1 reaches OCR1A (datasheet ATmega32 page 112)

  // Timer 1 will count from bottom to top every machine cycle and store it in the TCNT1 register. 
  // When it reaches the top, it resets automatically.

  ICR1 = 16000; // F_CPU = 16Mhz => T = 1/16Mhz = 0.0625us
                // T = total counts * 0.0625us
                // period T 1000us => total counts = 1000/0.0625 = 16000
  OCR1A = 16000/2; // Set 50% duty cycle => 500us high and 500us low
  // PWM waveform generation using OCR and ICR example image: https://www.arxterra.com/wp-content/uploads/2018/05/arduino_120.png
}


void timer2Init() {
  TCCR2A = 0;
  TIMSK2 = 0;
  TCNT2 = 0;

  DDRB |= (1 << PB3);

  TCCR2A |= (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);
  TIMSK2 |= (1 << OCIE2A);
  
  OCR2A = 128;

}

void startTimer1(){
  TCCR1B |= (1<<CS10);
}

void startTimer2() {
  TCCR2B |= (1 << CS22);
}

void stopTimer1(){
  TCCR1B &= ~(1<<CS10);
}

void stopTimer2() {
  TCCR2B &= ~(1 << CS22);
}

long caculate_pulse(long distance){
  long degree = (distance*360)/30;
  return round((degree)/0.9);
}

ISR (TIMER1_COMPA_vect) {
  if(top>=caculate_pulse(distanceX)){
    stopTimer1();
    top=0;
  }else{
    top += 1;
  }
  
} // Interupt when TCTN1 match the OCR1A

ISR(TIMER2_COMPA_vect) {
  if(top2>=caculate_pulse(distanceY)){
    stopTimer2();
    top2 = 0;
  }else{
    top2+=1;
  }
}

void menu(char* flag){
  if(flag=="x"){
    lcd.clear();
    lcd.print(">Set X");
    lcd.setCursor(0,1);
    lcd.print("Set Y");
  }
  if(flag=="y"){
    lcd.clear();
    lcd.print("Set X");
    lcd.setCursor(0,1);
    lcd.print(">Set Y");
  }
  if(flag=="enter"){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(">Enter");
  }
  if(flag=="set_x"){
    setXDistance();
  }
  if(flag=="set_y"){
    setYDistance();
  }
}

void setXDistance(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Distance: ");
  lcd.setCursor(9,0);
  lcd.print(String(xValue)+" mm");
}

void setYDistance(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Distance: ");
  lcd.setCursor(9,0);
  lcd.print(String(yValue)+" mm");
}

void checkButton(){

  if(digitalRead(set)==1){
    delay(200);
    Serial.println("set");
    
    if(currentMenu==""){
      menu("x");
      currentMenu = "x";
    }
    else if(currentMenu=="x"){
      menu("set_x");
      currentMenu = "set_x";
      upDownFunc = "value";
    }
    else if(currentMenu=="y"){
      menu("set_y");
      currentMenu = "set_y";
      upDownFunc = "value";
    }
    else if(currentMenu=="set_x"){
      menu("x");
      currentMenu = "x";
      upDownFunc = "normal";
    }
    else if(currentMenu=="set_y"){
      menu("y");
      currentMenu = "y";
      upDownFunc = "normal";
    }
    else if(currentMenu=="enter"){
      controlMotor();
    }
  }

  if(upDownFunc=="normal"){

    if(digitalRead(up)==1){
      delay(200);
      Serial.println("up");
      menuState+=1;
    }

    if(digitalRead(down)==1){
      delay(200);
      Serial.println("down");
      menuState-=1;
    }

  }
  else if(upDownFunc=="value"){

    if(currentMenu=="set_x"){
      if(digitalRead(up)==1){
        delay(200);
        Serial.println("up");
        if(xValue>=230){
          xValue=230;
        }else{
          xValue+=5;
        }
        menu("set_x");
      }

      if(digitalRead(down)==1){
        delay(200);
        if(xValue<=0){
          xValue=0;
        }else{
          xValue-=5;
        }
        menu("set_x");
      }
    }

    if(currentMenu=="set_y"){
      if(digitalRead(up)==1){
        delay(200);
        Serial.println("up");
        if(yValue>=230){
          yValue=230;
        }else{
          yValue+=5;
        }
        menu("set_y");
      }

      if(digitalRead(down)==1){
        delay(200);
        if(yValue<=0){
          yValue=0;
        }else{
          yValue-=5;
        }
        menu("set_y");
      }
    }

  }

}

void controlMotor(){

  if(xValue!=prevXvalue){
    distanceX = xValue - prevXvalue;

    if(distanceX < 0){
      digitalWrite(dirX,LOW);
      distanceX*=-1;
    }else{
      digitalWrite(dirX,HIGH);
    }

    startTimer1();

    prevXvalue = xValue;
  }

  if(yValue!=prevYvalue){
    distanceY = yValue - prevYvalue;

    if(distanceY < 0){
      digitalWrite(dirY,HIGH);
      distanceY*=-1;
    }else{
      digitalWrite(dirY,LOW);
    }

    startTimer2();

    prevYvalue = yValue;
  
  }


}

void updateMenu(){
  if(menuState<=-2){
    menuState = -2;
  }else if(menuState>=0){
    menuState = 0;
  }

  if(menuState!=prevMenuState && menuState==-1 ){
    menu("y");
    currentMenu = "y";
    prevMenuState = menuState;
  }
  if(menuState!=prevMenuState && menuState==0 ){
    menu("x");
    currentMenu = "x";
    prevMenuState = menuState;
  }
  if(menuState!=prevMenuState && menuState==-2 ){
    menu("enter");
    currentMenu = "enter";
    prevMenuState = menuState;
  }
}

void setup(){
  timer1Init();
  timer2Init();
  
  sei();

  pinMode(set,INPUT);
  pinMode(up,INPUT);
  pinMode(down,INPUT);

  pinMode(enaX,OUTPUT);
  pinMode(enaY,OUTPUT);
  pinMode(dirX,OUTPUT);
  pinMode(dirY,OUTPUT);

  digitalWrite(enaX,LOW);
  digitalWrite(enaY,LOW);

  lcd.init();
  lcd.backlight();
  lcd.print("STEP MOTOR");

}


void loop() {
  checkButton();
  updateMenu();
}

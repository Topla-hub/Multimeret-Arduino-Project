#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  

// inductance meter constants
double dPulseWid = 0.0;
double dFreaq = 0.0;
double dCap = 2.E-6;
double dInd = 0.0;
int indApplyPulsePin = 13;
int indReadPulsePin = 10;
int indLEDpin = 12;

// ammeter constants
int ammAnalogReading = 0;
float fVoltage = 0.0;
float fAmps = 0.0;
float fReq = 1.2; // equivalent resistance (low resistance resistor + jumper wires)
int ammAnalogInpPin = A0;
int ammLEDpin = 3;

// voltmeter constants
int vmtrAnalogPin = A1;
int vmtrAnalogReading = 0;
float fVmtrVout = 0.0;
float fVin = 0.0;
float R1 = 101000.0;
float R2 = 9760.0;
int volLEDpin = 6;

// ohmmeter variables
int ohmmtrAnalogReading = 0;
int ardVin = 5;
float ohmmtrVout = 0;
float ohmmtrR1 = 3200;
float resR = 0;
float ohmDummy = 0;
int ohmmtrAnalogPin = A2;
int ohmLEDpin = 5;

//light intensity constants
int liAnalogPin = A3;
int liAnalogReading = 0;
double liVout = 0.0;
double liR1 = 4210;
double liR2 = 0.0;
int liMaxR = 150000;
int liMinR = 0;
int bright = 0;


//thermometer constants
int thmeterAnalogPin = A6;
int thmeterAnalogReading = 0;
double thR1 = 1130;
double thLogR2 = 0.0;
double thR2 = 0.0;
double Temperature = 0.0;
double c1 = 1.009249522e-03;
double c2 = 2.378405444e-04;
double c3 = 2.019202697e-07;


// capacitance meter constants
int capAnalogPin = A7;
int capSupPin = 8;
int capDisPin = 9;
float capR1 = 9780.0;
unsigned long capStTime = 0.0;
unsigned long capElTime = 0.0;
float uFarads = 0.0;
int capLEDpin = 11;

// button constants 
int buttonPin = 2;
volatile unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 250;
 
int mode = 0;

void setup()
{
  lcd.init();
  lcd.clear();         
  lcd.backlight();

  // inductance meter setup
  pinMode(indReadPulsePin, INPUT);
  pinMode(indApplyPulsePin, OUTPUT);

  // ammeter setup
  pinMode(ammAnalogInpPin, INPUT);
  pinMode(ammLEDpin,OUTPUT);
  
  // voltmeter setup
  pinMode(vmtrAnalogPin, INPUT);

  // ohmmeter setup
  pinMode(ohmmtrAnalogPin, INPUT);
  
  // light intensity meter setup
  pinMode(liAnalogPin, INPUT);

  // thermometer setup
  pinMode(thmeterAnalogPin, INPUT);

  // capacitance meter setup
  pinMode(capAnalogPin ,INPUT);
  pinMode(capSupPin, OUTPUT);
  digitalWrite(capSupPin, LOW);

  // button setup
  pinMode(buttonPin, INPUT_PULLUP);

  // attach interrupt to button 
  attachInterrupt(digitalPinToInterrupt(buttonPin), changeMode, FALLING);

  delay(100);
}

void loop() 
{
  if (mode == 0) // inductance meter
  {
    lcd.clear();
    turnOff();
    digitalWrite(indLEDpin, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("INDUCTANCE: ");

    // charge inductor
    digitalWrite(indApplyPulsePin, HIGH);
    delay(5);
    digitalWrite(indApplyPulsePin, LOW);

    // measure half width of period
    delayMicroseconds(15);
    dPulseWid = pulseIn(indReadPulsePin, HIGH, 5000);

    if (dPulseWid > 0.1)
    {
      dPulseWid *= 2;
      dFreaq = 1.E6 / dPulseWid;
      dInd = 1. / (4.0 * PI * PI * dFreaq * dFreaq * dCap); // insert values in formula
      dInd *= 1E6;

      // lcd print
      lcd.setCursor(0, 1);
      lcd.print("uH: ");
      lcd.print(dInd);
      lcd.print("    "); 
      delay(100); 
    }

    delay(500); 
  }

  if(mode == 1) // ammeter
  {
    lcd.clear();
    turnOff();
    digitalWrite(ammLEDpin, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("AMMETER: ");

    ammAnalogReading = analogRead(ammAnalogInpPin);

    // scale analog reading to 0 - 5V
    fVoltage = ammAnalogReading * 5.0 / 1024.0;
    fAmps = fVoltage/fReq; 

    // lcd print
    lcd.setCursor(0, 1);
    lcd.print("amps: ");
    lcd.print(fAmps);
    lcd.print("    ");  
 
    delay(500);   
  }

  if(mode == 2) // voltmeter
  {
    lcd.clear();
    turnOff();
    digitalWrite(volLEDpin, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("VOLTMETER: ");
    // read analog input voltage
    vmtrAnalogReading = analogRead(vmtrAnalogPin);
    // scale analog reading to 0 - 5V
    fVmtrVout = vmtrAnalogReading * 5.0 / 1024.0;
    fVin = fVmtrVout * (R1 + R2) / R2; // insert values in formula

    if(fVin < 0.1) fVin = 0.0; 

    // lcd print
    lcd.setCursor(0, 1);
    lcd.print("Volts: ");
    lcd.print(fVin);
    lcd.print("    ");  

    delay(500);
  }
   
if (mode == 3) // Ohmmeter mode
{
  lcd.clear();
  turnOff();
  digitalWrite(ohmLEDpin,HIGH);
  lcd.setCursor(0, 0);
  lcd.print("OHMMETER: ");
  // analog reading -> voltage -> resistance
  ohmmtrAnalogReading = analogRead(ohmmtrAnalogPin);
  if(ohmmtrAnalogReading)
  {
    ohmDummy = ohmmtrAnalogReading * ardVin;
    ohmmtrVout = (ohmDummy)/1024.0;
    ohmDummy = (ardVin/ohmmtrVout) - 1;
    resR = ohmmtrR1 * ohmDummy;
    lcd.setCursor(0, 1);
    lcd.print(resR);
  } 
  delay(500);   
}

  
  if(mode == 4) // light intensivity meter
  {
    lcd.clear();
    turnOff();
    lcd.setCursor(0, 0);
    lcd.print("LIGHT ");
    // calculate voltage from analog reading and current resistance from voltage
    liAnalogReading = analogRead(liAnalogPin);
    liVout = liAnalogReading * 5.0 / 1024.0;
    double k = (5.0/liVout) - 1; 
    liR2 = liR1 * k;
    double dFraq = liR2 / liMaxR;
    // calculate precentage current resistance is of maximum resistance
    bright = 100 * dFraq;
    if(bright > 100) bright = 100;
    if(bright < 0) bright = 0;
    int outp = 100 - bright;
    
    // lcd print
    lcd.setCursor(0, 1);
    lcd.print("INTENSITY: ");
    lcd.print(outp);
    lcd.print("    ");  
    
    delay(500);

  }

  if(mode == 5) // thermometer
  {
    lcd.clear();
    turnOff();
    lcd.setCursor(0, 0);
    lcd.print("TEMPERATURE: ");
    thmeterAnalogReading = analogRead(thmeterAnalogPin);
    for(int i = 0 ; i < 500 ; i++)
    {
      // calculate mean value of analog reading
      thmeterAnalogReading = (thmeterAnalogReading + analogRead(thmeterAnalogPin)) / 2;   
    }
    // calculate resistance and temperature from it
    thR2 = thR1 * (1024.0  / (double)thmeterAnalogReading - 1.0);
    thLogR2 = log(thR2);
    Temperature = (1.0 / (c1 + c2 * thLogR2 + c3 * thLogR2 * thLogR2 * thLogR2));
    Temperature -= 273.15;
    // lcd print
    lcd.setCursor(0, 1);
    lcd.print(Temperature);
    lcd.print("    ");  
    delay(500);
  }

  if (mode == 6) // Capacitance meter
  {
      lcd.clear();
      turnOff();
      // save old measurement in variable and clear display
      float oldF = uFarads;
      lcd.setCursor(0, 0);
      lcd.print("CAPACITANCE: ");
      lcd.setCursor(0, 1);
      lcd.print(uFarads);

      while (mode == 6) 
      {
        // start charging capacitor and start timer
        digitalWrite(capLEDpin, HIGH);
        digitalWrite(capSupPin, HIGH);
        capStTime = micros();
        //wait until 63% is charged
        while (mode == 6 && analogRead(capAnalogPin) < 648) 
        {
            if (mode != 6) 
            {
                digitalWrite(capSupPin, LOW);
                digitalWrite(capLEDpin, LOW);
                return;
            }
        }

        capElTime = micros() - capStTime;
        uFarads = ((float)capElTime / capR1);
        digitalWrite(capSupPin, LOW);
        pinMode(capDisPin, OUTPUT);
        digitalWrite(capDisPin, LOW);
        // discharge capacitor
        while (mode == 6 && analogRead(capAnalogPin) > 0) 
        {
          //stop if button is clicked during that time
            if (mode != 6) 
            {
                digitalWrite(capDisPin, LOW);
                pinMode(capDisPin, INPUT);
                digitalWrite(capLEDpin, LOW);
                return;
            }
        }

        pinMode(capDisPin, INPUT);

        if (uFarads != oldF) 
        {
          //LCD print new value if it is really new (capacitor may be charge before starting measurment
          // which will lead to incorrect answer on first try but second try will always give correct result
          // because capacitor is discharged)
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("CAPACITANCE: ");
            lcd.setCursor(0, 1);
            lcd.print(uFarads);
            oldF = uFarads; 
        }

        delay(500); 
      }

    digitalWrite(capLEDpin, LOW);
  }

}

void changeMode() {
  // changes mode INTERRUPT fuction
  if ((millis() - lastDebounceTime) > debounceDelay) {
    mode++;
    if (mode > 6) {
      mode = 0;
    }
    lastDebounceTime = millis();
  }
}

void turnOff()
{
  //turns off every LED
  digitalWrite(ammLEDpin, LOW);
  digitalWrite(ohmLEDpin, LOW);
  digitalWrite(volLEDpin, LOW);
  digitalWrite(indLEDpin, LOW);
  digitalWrite(capLEDpin, LOW);
}
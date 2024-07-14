#include <LiquidCrystal_I2C.h>
#include <dht11.h>
#include <Servo.h>
#include <Keypad.h>
#include <Password.h>

#define BUZZER_PIN 30
#define SEASON_CHANGER_SWITCH_PIN 27
#define INCREASE_SETPOINT_BUTTON_PIN 28
#define DECREASE_SETPOINT_BUTTON_PIN 29
#define WINDOW1_SERVO_PIN 50
#define WINDOW2_SERVO_PIN 52
#define GASVALVE_SERVO_PIN 11
#define DOOR_SERVO_PIN 12
#define LEDS_ON_BUTTON 26
#define DHT11_PIN 22
#define LED1 4
#define LED2 3
#define LED3 2
#define LED1_BUTTON 23
#define LED2_BUTTON 24
#define LED3_BUTTON 25
#define MOTOR1_SPEED 10
#define MOTOR1_PIN1 9
#define MOTOR1_PIN2 8
#define MOTOR2_PIN1 7
#define MOTOR2_PIN2 6
#define MOTOR2_SPEED 5

const int FIRESENSOR_PIN = A7;
const int MQ2_PIN = A1;
const int RAINSENSOR_PIN = A0;

byte snowflake[8] = {
  B00000,
  B01010,
  B10101,
  B01110,
  B11111,
  B01110,
  B10101,
  B01010
};

byte sun[8] = {
  B00000,
  B10001,
  B01010,
  B01110,
  B11011,
  B01110,
  B01010,
  B10001
};

LiquidCrystal_I2C lcd(0x27, 16, 2);
dht11 DHT11;
Servo servoWindow1;
Servo servoWindow2;
Servo gasValve;
Servo servoDoor;

const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {53, 51, 49, 47};
byte colPins[COLS] = {45, 43, 41};

Keypad customKeypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


const int MAX_DELAY = 50;
const float setpointStep = 0.5;

float currentTemperature;
float currentHumidity;
float targetTemperature = 25.0;

int ledOneBrightness = 50;
int ledTwoBrightness = 50;
int ledThreeBrightness = 50;
int rainSensorValue;
int gasSensorValue;
int fireSensorValue;
int gasFlag;
int rainFlag;
int fireFlag;
int defaultDisplayTime;

const int passwordLength = 5;
byte passwordArrayCount = 0;
char Data[passwordLength];
char Master[passwordLength] = "2024";
char customKey;

bool doorIsLocked = true;
bool rainDetectedDisplayed = false;
bool lastSeasonSwitchState = false;
bool currentSeasonSwitchState;
bool increaseSetpointButton;
bool decreaseSetpointButton;
bool ledOneButtonState;
bool ledTwoButtonState;
bool ledThreeButtonState;
bool ledsOnButtonState;
bool lastStateIncreaseSetpointButton;
bool lastStateDecreaseSetpointButton;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SEASON_CHANGER_SWITCH_PIN, INPUT);
  pinMode(RAINSENSOR_PIN, INPUT);
  pinMode(MQ2_PIN, INPUT);
  pinMode(FIRESENSOR_PIN, INPUT);
  pinMode(INCREASE_SETPOINT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DECREASE_SETPOINT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MOTOR1_PIN1, OUTPUT);
  pinMode(MOTOR1_PIN2, OUTPUT);
  pinMode(MOTOR1_SPEED, OUTPUT);
  pinMode(MOTOR2_PIN1, OUTPUT);
  pinMode(MOTOR2_PIN2, OUTPUT);
  pinMode(MOTOR2_SPEED, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED1_BUTTON, INPUT_PULLUP);
  pinMode(LED2, OUTPUT);
  pinMode(LED2_BUTTON, INPUT_PULLUP);
  pinMode(LED3, OUTPUT);
  pinMode(LED3_BUTTON, INPUT_PULLUP);
  pinMode(LEDS_ON_BUTTON, INPUT_PULLUP);

  servoWindow1.attach(WINDOW1_SERVO_PIN);
  servoWindow2.attach(WINDOW2_SERVO_PIN);
  gasValve.attach(GASVALVE_SERVO_PIN);
  servoDoor.attach(DOOR_SERVO_PIN);
  servoClose(servoWindow1);
  servoClose(servoWindow2);
  servoClose(servoDoor);
  servoOpen(gasValve);

  Serial.begin(9600);
  lcd.init();
  lcd.createChar(0, snowflake);
  lcd.createChar(1, sun);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SmartHome System");
  lcd.setCursor(0, 1);
  lcd.print("Initializat");
  delay(5000);
  lcd.clear();
}

int temperatureRead() {
  int sensorValue = DHT11.read(DHT11_PIN);
  float temperature = DHT11.temperature;
  return temperature;
}

int humidityRead() {
  int sensorValue = DHT11.read(DHT11_PIN);
  float humidity = DHT11.humidity;
  return humidity;
}

void autoSensor(){
  gasSensorValue = analogRead(MQ2_PIN);
  rainSensorValue = analogRead(RAINSENSOR_PIN);
  fireSensorValue = analogRead(FIRESENSOR_PIN);

	if(gasSensorValue >= 400){
		gasFlag = 1;
	}else if(gasSensorValue < 100){
    gasFlag = 0;
	}
	
	if(rainSensorValue <= 600 && !rainDetectedDisplayed){
		rainFlag = 1;
		servoClose(servoWindow1); 
		servoClose(servoWindow2);
		rainDetectedDisplayed = true;
	}

	if(fireSensorValue <= 600){
		fireFlag = 1;
    servoOpen(servoWindow1);
    servoOpen(servoWindow2);
	}else if(fireSensorValue > 900){
		fireFlag = 0;
	}

  if(gasFlag || fireFlag){
    servoClose(gasValve);
  }else
  {
    servoOpen(gasValve);
  }
}

void servoOpen(Servo servo) {
  int pos;
  for (pos = 10; pos <= 95; pos += 5) {
    servo.write(pos);
  }
}

void servoClose(Servo servo) {
  int pos;
  for (pos = 95; pos >= 10; pos -= 5) {
    servo.write(pos);
  }
}

void modifyBrightnessLedOne(){
  ledOneBrightness += 5;
}
void modifyBrightnessLedTwo(){
  ledTwoBrightness += 5;
}
void modifyBrightnessLedThree(){
  ledThreeBrightness += 5;
}

void loop() {   
  if (!doorIsLocked) {
    autoSensor();
    closeDoor();
    // Controlul sistemului de iluminare
    ledOneButtonState = digitalRead(LED1_BUTTON);
    ledTwoButtonState = digitalRead(LED2_BUTTON);
    ledThreeButtonState = digitalRead(LED3_BUTTON);
    ledsOnButtonState = digitalRead(LEDS_ON_BUTTON);
    if(!ledOneButtonState){
      modifyBrightnessLedOne();
    }
    if(!ledTwoButtonState){
      modifyBrightnessLedTwo();
    }
    if(!ledThreeButtonState){
      modifyBrightnessLedThree();
    }
    if(ledOneBrightness > 255){
      ledOneBrightness = 0;
    }
    if(ledTwoBrightness > 255){
      ledTwoBrightness = 0;
    }
    if(ledThreeBrightness > 255){
      ledThreeBrightness = 0;
    }
    if(!ledsOnButtonState){
      ledOneBrightness = 255 * 0.7;
      ledTwoBrightness = 255 * 0.7;
      ledThreeBrightness = 255 * 0.7;
    }
    analogWrite(LED1, ledOneBrightness);
    analogWrite(LED2, ledTwoBrightness);
    analogWrite(LED3, ledThreeBrightness);
    
    // Controlul anotimpului si setpoint-ului
    currentSeasonSwitchState = digitalRead(SEASON_CHANGER_SWITCH_PIN);
    increaseSetpointButton = digitalRead(INCREASE_SETPOINT_BUTTON_PIN);
    decreaseSetpointButton = digitalRead(DECREASE_SETPOINT_BUTTON_PIN);
    if(increaseSetpointButton != lastStateIncreaseSetpointButton){
      if(!increaseSetpointButton){
        targetTemperature += setpointStep;
      }
      lastStateIncreaseSetpointButton = increaseSetpointButton;
    }
    if(decreaseSetpointButton != lastStateDecreaseSetpointButton){
      if(!decreaseSetpointButton){
        targetTemperature -= setpointStep;
      }
      lastStateDecreaseSetpointButton = decreaseSetpointButton;
    }

    // Controlul sistemului de climatizare, motoare
    digitalWrite(MOTOR1_PIN1, HIGH);
    digitalWrite(MOTOR1_PIN2, LOW);
    digitalWrite(MOTOR2_PIN1, LOW);
    digitalWrite(MOTOR2_PIN2, HIGH);
    if(!currentSeasonSwitchState){	
      if(targetTemperature - currentTemperature > 5.0){
        analogWrite(MOTOR1_SPEED, 120);
      } else if(targetTemperature - currentTemperature >= 2.0 && targetTemperature - currentTemperature <= 5.0){
        analogWrite(MOTOR1_SPEED, 80);
      } else if(targetTemperature - currentTemperature < 2.0 && targetTemperature - currentTemperature >= 1.0){
        analogWrite(MOTOR1_SPEED, 50);
      } else{
        analogWrite(MOTOR1_SPEED, 0);
      }
    }
    if(currentSeasonSwitchState){	
      if(currentTemperature - targetTemperature > 5.0){
        analogWrite(MOTOR2_SPEED, 120);
      } else if(currentTemperature - targetTemperature >= 2.0 && currentTemperature - targetTemperature <= 5.0){
        analogWrite(MOTOR2_SPEED, 80);
      } else if(currentTemperature - targetTemperature < 2.0 && currentTemperature - targetTemperature >= 1.0){
        analogWrite(MOTOR2_SPEED, 50);
      } else{
        analogWrite(MOTOR2_SPEED, 0);
      }
    }
    // Citirea senzorului DHT11 si afisarea pe ecranul LCD al informatiilor relevante
      currentTemperature = temperatureRead();
      currentHumidity = humidityRead();

      if (defaultDisplayTime > 500) {
        lcd.clear();
        noTone(BUZZER_PIN);

        if(currentSeasonSwitchState != lastSeasonSwitchState){
          if(!currentSeasonSwitchState){
            lcd.setCursor(5, 0);
            lcd.print("MOD");
            lcd.setCursor(6, 1);
            lcd.print("IARNA");
          }else{
            lcd.setCursor(5, 0);
            lcd.print("MOD");
            lcd.setCursor(6, 1);
            lcd.print("VARA");
          }
          lastSeasonSwitchState = currentSeasonSwitchState;

        } else if (rainFlag) {
          tone(BUZZER_PIN, 1000, 5000);
          lcd.setCursor(4, 0);
          lcd.print("Ploaie");
          lcd.setCursor(3, 1);
          lcd.print("detectata!!");
          rainFlag = 0;

        } else if (gasFlag) {
          tone(BUZZER_PIN, 1000, 5000);
          lcd.setCursor(5, 0);
          lcd.print("Gaze");
          lcd.setCursor(2, 1);
          lcd.print("detectate!!");

        } else if (fireFlag) {
          tone(BUZZER_PIN, 1000, 5000);
          lcd.setCursor(2, 0);
          lcd.print("Incendiu");
          lcd.setCursor(2, 1);
          lcd.print("detectat!!");

        } else {
          lcd.setCursor(0, 0);
          lcd.print("Mod:");
          lcd.write(byte(currentSeasonSwitchState));
          lcd.print(" SP:");
          lcd.print(targetTemperature, 1);
          lcd.print("C");
          lcd.setCursor(0, 1);
          lcd.print("T:");
          lcd.print(currentTemperature, 1);
          lcd.print("C  ");
          lcd.print("H:");
          lcd.print(currentHumidity, 1);
          lcd.print("%");
        }
        defaultDisplayTime = 0;
      }
      
      if(rainSensorValue > 900 && rainDetectedDisplayed){
        rainDetectedDisplayed = false;
        servoOpen(servoWindow1);
        servoOpen(servoWindow2);
      }

      defaultDisplayTime += MAX_DELAY;
      delay(MAX_DELAY);
  } else {
    openDoor();
  }
}

void clearData()
{
  while (passwordArrayCount != 0)
  {
    Data[passwordArrayCount--] = 0;
  }
  return;
}

void closeDoor(){
  customKey = customKeypad.getKey();
  if (customKey == '#')
  {
    lcd.clear();
    servoClose(servoDoor);
    lcd.print("USA INCHISA!!");
    delay(3000);
    doorIsLocked = true;
  }
}

void openDoor()
{
  lcd.setCursor(0, 0);
  lcd.print("Introdu parola:");
  customKey = customKeypad.getKey();
  if (customKey)
  {
    Data[passwordArrayCount] = customKey;
    lcd.setCursor(passwordArrayCount, 1);
    lcd.print(Data[passwordArrayCount]);
    passwordArrayCount++;
  }

  if (passwordArrayCount == passwordLength - 1)
  {
    if (!strcmp(Data, Master))
    {
      lcd.clear();
	    servoOpen(servoDoor);
      tone(BUZZER_PIN, 1000, 1000);
      lcd.print("USA DESCHISA!!");
      doorIsLocked = false;
      delay(3000);
    }
    else
    {
      lcd.clear();
      lcd.print("Parola gresita");
      delay(1000);
      doorIsLocked = true;
    }
    clearData();
  }
}
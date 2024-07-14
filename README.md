# Smart-Home-System-Using-Arduino
This project was created as a final exam requirement for a Bachelor's degree in Computer Science. This system aims to enhance home automation, security, and energy efficiency by leveraging the capabilities of Arduino microcontrollers and various sensors and actuators. This project uses an Arduino Mega 2560 board.

**Key Features:**

**LCD Display:** A LCD Display 16x2, based on the Hitachi HD44780 LCD controller, is implemented in this project. By default, it displays the current temperature, humidity, target temperature and current season for the system. When any sensor is triggered, an alert is displayed on the LCD and the buzzer is turned on.

**HVAC System:** Uses DHT11 sensor for real-time monitoring and automatic adjustment of indoor climate. A thermostat system was also developed, using the information from DHT11. There are 2 3-6V DC Motors added, one for cooling, one for heating. They are controlled using a L298N module, H-Bridge based. Using a slide-switch for setting the season (Summer/Winter), each motor is controlled based on the current season (Heating motor in Winter mode and cooling motor in Summer mode, respectively). There are 3 temperature intervals in which the motor is running, at different speeds (using PWM functionality from the Arduino board).

**Gas and Fire Detection:** MQ2 gas sensor and fire sensor are used to detect hazards and trigger safety measures. A gas valve is implemented in this project (based on a servomotor) to be closed in case there is a gas leakage or fire warning. Windows are also opened (also based on 2 servomotors) when a fire warning is triggered.

**Rain Detection:** Using a FR-04 base for detecting raindrops, it automatically closes the 2 windows when they are detected, the signal being sent by the rain sensor.

**Lighting Control:** Three LEDs are added in this project to be controlled with buttons. They are dimmable, holding the button for each LED down increasing the brightness of that LED. There is also a button implemented for setting the brightness to 70% for each LED.

**Security System:** A 4x3 Keypad was implemented to lock/unlock the main door of the Smart House. The password can be set through code, by default is "2024". When the correct password is introduced, the door opens. Else, "Wrong password" will be displayed on the LCD. When the door is unlocked, press "#" to lock the door. A buzzer is implemented to alert the user of any sensor trigger or when the door is locked/unlocked.

**Future Enhancements:**
Bluetooth/Wireless Connectivity: Smartphone control via Bluetooth or Wireless. An Android app also needs to be developed for the control of this smart home system.

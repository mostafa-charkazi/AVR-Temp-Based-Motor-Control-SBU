# AVR Temperature-Based Motor Control System

A university project developed using the ATmega32 microcontroller. This project measures temperature using an LM35 sensor and adjusts the speed of a DC motor accordingly using PWM. It also includes a password-protected menu system, an LCD interface, and a 4-digit 7-segment display.

## Features
- 🔐 Login system via keypad (74C922)
- 🌡️ Temperature reading via LM35 sensor
- ⚙️ PWM-based DC motor speed control
- 📟 20x4 LCD user interface
- ⏰ Internal real-time clock using Timer
- 🔢 4-digit 7-segment display driven by 7448
- 🧮 Adjustable threshold and motor speed
- 🛠️ Written in C using CodeWizardAVR + AVR-GCC

## Hardware
- Microcontroller: ATmega32 (2 MHz)
- LCD 20x4 (connected to PORTB)
- Keypad via 74C922 (connected to PORTD)
- Temperature Sensor: LM35 (connected to ADC0)
- 4-digit Common Cathode 7-Segment (via 7448 and PORTC)
- DC Motor with PWM via Timer2
- Pushbuttons for interaction

## How It Works
1. On boot, project displays student info.
2. Shows real-time temperature and system clock.
3. On pressing 'C' key, a login screen appears.
4. If credentials are correct:
   - User can access menu to set:
     - Temperature threshold
     - Motor speed
     - Clock settings
5. Motor speed increases every 15s if temperature exceeds threshold.

## Authors
- Mostafa Charkazi – [401249015]
- Meysam Sadeghi – [401249036]
- Ali Asghar Bararjanpour – [401249008]

## License
This project is for educational purposes only. No commercial use permitted.

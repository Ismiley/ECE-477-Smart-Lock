# SMARTLOCK

## Goal
The goal of this project is to create a smart lock system that allows users to unlock a door by using their phones WiFi. In addition to this feature the lock has a keypad as backup in case the user loses their phone. This smart lock is designed for rental property owners to easily grant access to renters for short periods of time.\
The lock comes in two parts: one side is on the outside of the door and one side is inside. The outside is where all the user interaction occurs. We will be designing a 3D printed casing to hold our WiFi module, multiplexed Keypad, and mechanical lock. The keypad is added as a backup system if a tenant loses their phone but remembers the code. The WiFi module communicates via UART, and our 16 key multiplexed keypad will use GPIO.\
On the inside is where most of our electronics will be. Another 3D printed case will house all the parts. The MCU will be connected to a PCB where all of our modules will connect to as well. Our WiFi module will be connected as well using GPIO pins on our microcontroller. The WiFi module will be used to wirelessly control the lock via the app. We also will use the WiFi module to send notifications to a web server (which we will develop and deploy), and this web server will then send these notifications to the property owner. We will have our motor to allow for the electronic un/lock. For power we will be using a 26800mAh usb power bank that can be charged via USB-C.\
We will also develop a smartphone application that lets the user and the property owner communicate with the Smart Lock via WiFi. The user will be able to save the unlock code in the app. When they are connected to the same WiFi that the SmartLock is connected to, they will be able to press a button on the app that unlocks the door. The property owner can use the app to reset the unlock code and set the expiration date of this code, which is also done through Wifi.

## PROJECT SPECIFIC DESIGN REQUIREMENTS (PSDRS)
- PSDR #1 (Hardware): An ability to interface a microcontroller with a 12 key multiplexed keypad via GPIO to allow users to enter a predetermined code to unlock the door.
- PSDR #2 (Hardware): An ability to interface a microcontroller using I2C with a light-driven proximity sensor to determine when the door is open.
- PSDR #3 (Hardware): An ability for the microcontroller to interface with a WiFi module via the UART communication protocol in order to send notification to the property owner.
- PSDR #4 (Software): An ability to present an intuitive user interface, in the form of a smart phone application, that allows both the renter and property owner to send and receive data, through Wi-Fi, to the Smart Lock.
- PSDR #5 (Software): An ability to have a working web server that provides a mechanism for the Smart Lock to send text messages to the property owner.

## Contributors
- Ismail Husain
- John Bensen (WiFi Interfacing)

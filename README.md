# BluesNoteCard-TempSensor

This device is used to monitor a vacant house during a vacation. It reports temperature, humidity, mains power and battery life over cellular. There is an OLED screen to display info only while on USB power.The box runs on USB power unless there is a power outage. Data is uploaded to the free version of Ubidots. Daily reports and important events (low temp, power outage, etc) are sent to text messages and email.

A custom hat/shield contains all the extra components and plugs into the headers on the Notecard. See Schematic.
Components:

* Blues Notecard-F & antenna
* ESP32-Feather
* Htu31d Temp and Humidity I2C Sensor (www.adafruit.com/product/5580)
* 1200mAh Lipo battery
* MAX17048 Lipo Fuel Gauge (www.adafruit.com/product/4832)
* I2C OLED
* DPDT power switch
* 3D Printed enclosure

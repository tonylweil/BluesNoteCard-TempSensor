# BluesNoteCard-TempSensor

This device reports temperature, humidity, mains power and battery life in a house during vacations in winter over cellular. There is an OLED screen to display info only while on USB power.The box runs on USB power unless there is a power outage. Data is uploaded to the free version of Ubidots. Ubidots has ev
Daily reports and important events (low temp, power outage, etc) are sent to text messages and email.

Components:

Blues Notecard-F

ESP32-Feather

Htu31d Temp and Humidity I2C Sensor 

500mah Lipo battery

LC709203F Lipo Fuel Gauge (adafruit)

OLED

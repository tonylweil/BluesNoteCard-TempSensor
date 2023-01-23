// Notecard with HTU31-D Temp/Humidity sensor
// output to serial monitor

/*
 * Sleepy Sensor Application
 *
 * The goal of this application is to show the elements required to construct an
 * application which leverages the capabilities of the Notecard to minimize
 * battery consumption by controlling the wake time of the host MCU.
 *
 * Use the following diagram to wire up the circuit. Note the Notecard's `ATTN`
 * pin (exposed via the Notecarrier) is wired to the enable pin of the host MCU.
 *  ________________________________________
 * |  ____________________________________  |               ____________________
 * | |                                    | |              /
 * | |        _________________           | |             |  O
 * | |       |O|             |O|          | |             |     (Notecarrier-AL)
 * | --> SDA-|*  ___________  *|-21       | |       VUSB>-|[]
 * ----> SCL-|* | ESPRESSIF | *|-TX       | |  ----> BAT>-|[]
 *        14-|* |ESP32-WROOM| *|-RX       | |  |    MAIN>-|[]
 *        32-|* |CE         | *|-MI       | |  |     VIO>-|[]
 *        15-|* |           | *|-MO       | |  |          |[]
 *        33-|* |       ___ | *|-SCK      | |  |       V+-|[]
 *        27-|* |      |   || *|-A5    ---^-^--^----> GND-|[]
 *        12-|* |______|___|| *|-A4    |  | |  |       EN-|[]
 *        13-|*               *|-A3    |  | |  |      RST-|[]
 *       USB-|*               *|-A2    |  | |  |          |[]
 * -----> EN-|*         ----- *|-A1    |  | ---^----> SCL-|[]
 * | --> BAT-|*___      |   | *|-A0    |  -----^----> SDA-|[]
 * | |       |    |     ----- *|-GND <--       | --> ATTN-|[]
 * | |       |    |        _  *|-NC            | |  AUXEN-|[]
 * | |       |-----       |O| *|-3V            | |  AUXRX-|[]
 * | |       |      -----     *|-RST           | |  AUXTX-|[]
 * | |       |O___0_|___|_0___O|               | |   AUX1-|[]
 * | |                                         | |   AUX2-|[]
 * | |_________________________________________| |   AUX3-|[]
 * |_____________________________________________|   AUX4-|[]
 *                                                     RX-|[]
 *                                                     TX-|[]
 *                                                        |
 *                                                        |  O
 *                                                         \____________________
 *
 * NOTE: This sample is intended to compile for any Arduino compatible MCU
 * architecture and Notecard/Notecarrier combination. However, it was created
 * and tested using the Adafruit Huzzah32 and Notecarrier-AL.
 *
 * NOTE: This example has intentionally omitted error checking in order to
 * highlight the elements required to make a power-efficient application.
 */

//=================IMPORTANT: MUST WIRE Notecarrier ATTN pin to F_EN pin as in above drawing==============

// Notecard
#include <Notecard.h>
#define usbSerial Serial
#define productUID "com.xxxxxxxxxxx:tempsensor"
Notecard notecard;
// This period controls the waking frequency of your host MCU, and will have a
// direct impact on battery conservation. It should be used to strike a balance
// between battery performance and data collection requirements.
//static const size_t PERIOD_S = 1800; // 30 min
static const size_t PERIOD_S = 900; // 900 seconds = 15 min updates

#include <Wire.h>
#include "Adafruit_HTU31D.h"
Adafruit_HTU31D htu = Adafruit_HTU31D();

#include "Adafruit_MAX1704X.h"
Adafruit_MAX17048 maxlipo;

int powerPin = 14; //feather GPIO14, Note F_D5, Orange
int powerValue = 0;

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
//Adafruit_SSD1306 display(128, 64, &Wire);

float batteryValue = 0;

void setup() {
  usbSerial.begin(115200);
  usbSerial.println();
  usbSerial.println("==============================================================================");
  usbSerial.println("Version: Notecard_tempsensor_htu31d_power_batt-7_ATTN_OLED_USB_MAX_FINAL_AUX");
  usbSerial.println("==============================================================================");
  usbSerial.println();

  // Initialize Display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS); 
  // Initialize Max 17048 Lipo Fuel Gauge
  maxlipo.begin();
 // Initialize Notecard
  notecard.begin();
  notecard.setDebugOutputStream(usbSerial);
//initialize temp sensor
  htu.begin(0x40);  

 // Provide visual signal when the Host MCU is powered
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); 

  pinMode(powerPin, INPUT_PULLDOWN); // Power Sense pin - HIGH if USB power is on , 5V, no divider!

  // Configure Notecard to synchronize with Notehub periodically, as well as
  // adjust the frequency based on the battery level 
 {
 J * req = notecard.newRequest("hub.set");
 JAddStringToObject(req, "product", productUID);
 JAddStringToObject(req, "mode", "periodic");
 JAddStringToObject(req, "voutbound", "usb:15;high:15;normal:15;low:60;dead:0"); 
 JAddStringToObject(req, "vinbound", "usb:15;high:15;normal:15;low:60;dead:0");
 //JAddStringToObject(req, "vinbound", "usb:60;high:60;normal:60;low:480;dead:0");
 notecard.sendRequest(req);
 }

 // Optimize voltage variable behaviors for LiPo battery
  {
    J * req = notecard.newRequest("card.voltage");
    JAddStringToObject(req, "mode", "lipo");
    notecard.sendRequest(req);
  }

  // Set up Notecarrier AUX GPIO for monitoring USB power connected to AUX1
 {
 J *req = NoteNewRequest("card.aux");
 JAddStringToObject(req, "mode", "gpio");
 J *pins = JAddArrayToObject(req, "usage");
 JAddItemToArray(pins, JCreateString("input-pulldown")); // AUX1
 JAddItemToArray(pins, JCreateString("off")); // AUX2
 JAddItemToArray(pins, JCreateString("off")); // AUX3
 JAddItemToArray(pins, JCreateString("off")); // AUX4
 JAddBoolToObject(req, "sync", true);
 JAddStringToObject(req, "file", "power-outage.qo"); // is this necessary or should it be sensors.qo?
 NoteRequest(req);
 }

 
  //ReadData();
  powerValue = digitalRead(powerPin); // 0=OFF, 1=ON // AC Power Sensor
  sensors_event_t humidity, temp;
  htu.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data

  batteryValue = maxlipo.cellPercent();
  if (batteryValue > 100) { 
    batteryValue = 100;
    }

//SendData();  //Send data to Notehub
  usbSerial.println("=======================");
  usbSerial.println("Sending data to Notehub");
  usbSerial.println("=======================");
    {
    J * req = notecard.newRequest("note.add");
    JAddStringToObject(req, "file", "sensors.qo");
    JAddBoolToObject(req, "sync", true);
    J * body = JAddObjectToObject(req, "body");
    if (body)
    {
      JAddNumberToObject(body, "temp", temp.temperature * 1.8 + 32);
      JAddNumberToObject(body, "humidity", humidity.relative_humidity);
      JAddNumberToObject(body, "battery",batteryValue);
      JAddNumberToObject(body, "power", powerValue);
    }
    notecard.sendRequest(req);
  } 
  if(powerValue == 1) {
   //DisplayData();
   display.clearDisplay();
   display.setCursor(0,0);
   display.setTextSize(2);             // Draw 2X-scale text
   display.setTextColor(SSD1306_WHITE);

   display.print(F("Temp:"));
   display.print((temp.temperature)* 1.8 + 32,0);
   display.println(F(" F"));
   display.print(F("Hum :"));
   display.print(humidity.relative_humidity,0);
   display.println(F(" %"));
   display.print(F("Batt:"));
   display.print(batteryValue,0);
   display.println(F(" %"));
   display.print(F("AC  :"));
    if(powerValue == 1) {
     display.println("ON");
    }
    else {
     display.println("OFF");
    }
   display.display();
  }
}

void loop() {
  // Request sleep from loop to safeguard against tranmission failure, and
  // ensure sleep request is honored so power usage is minimized.
    {
      // Create a "command" instead of a "request", because the host
      // MCU is going to power down and cannot receive a response.
      J * req = NoteNewCommand("card.attn");
      JAddStringToObject(req, "mode", "sleep");
      JAddNumberToObject(req, "seconds", PERIOD_S);
      notecard.sendRequest(req);
    }
  // Wait 1s before retrying
 delay(1000);
   
}

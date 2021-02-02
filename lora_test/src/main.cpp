#include <Arduino.h>

void lora_tx();
void wind_m();

const byte lora_PNSS = 18;    // pin number where the NSS line for the LoRa device is connected.
const byte lora_PReset = 23;  // pin where LoRa device reset line is connected
//const byte lora_DIO0 = 26;
const byte interruptPin = 14;
const byte interruptPinW = 12;

const byte PLED1 = 25;        // pin number for LED on Tracker
int cnt=1;
int cnt_1H=0;
int cnt_24H=0;
int cnt_rain=0;
int cnt_wind=0;
int sekund = 1;
int calc=1;
int cnt_rain_cor = 0;
int adc_v2 = 0;
float windr = 0;
String Outstring="";  

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <SPI.h>
#include <sensor.h>
#include <LoRaTX.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_HTU21DF.h>

#define OLED_SDA 21
#define OLED_SCL 22 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
Adafruit_BMP085 bmp;
Adafruit_HTU21DF htu;

void IRAM_ATTR isr(){
  cnt_rain = cnt_rain + 1;
  digitalWrite(PLED1, HIGH);
  }

void IRAM_ATTR wind(){
  cnt_wind = cnt_wind + 1;
  digitalWrite(PLED1, HIGH);
  }

void setup() {
//initialize OLED
Wire.begin(OLED_SDA, OLED_SCL);
if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
Serial.println(F("SSD1306 allocation failed"));
for(;;); // Don't proceed, loop forever
}
if (!bmp.begin()){
  Serial.println(F("bmp180 - not ok"));
  while (1) 
  {
    Serial.println(F("bmp180 - ok"));
  }
  
}
if (!htu.begin()){
  Serial.println(F("htu - not ok"));
  while (1) 
  {
    Serial.println(F("htu - ok"));
  }
  
}  
pinMode (interruptPin, INPUT_PULLUP);
pinMode (interruptPinW, INPUT_PULLUP);

display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(0,0);
display.print("OK5TVR-6");
display.setTextSize(1);
display.setCursor(5,36);
display.print("WX test APRS LoRa");
display.display();
delay(3000);


pinMode(lora_PReset, OUTPUT);     // RFM98 reset line
  digitalWrite(lora_PReset, LOW);   // Reset RFM98
  pinMode (lora_PNSS, OUTPUT);      // set the slaveSelectPin as an output:
  digitalWrite(lora_PNSS, HIGH);
  pinMode(PLED1, OUTPUT);                                                          // for shield LED
  
  SPI.begin();                                                                     // initialize SPI:
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
   
   lora_ResetDev();                                                                 // Reset the device
   lora_Setup();                                                                    // Do the initial LoRa Setup

// LoRa frequency calculation (sample for 434.4 MHz): 
// ------------------------------------
// 434400000/61.03515625 = 71172096
// 71172096 (DEC) = 6C 99 99 (HEX)  
// 6C 99 99 (HEX) = 108 153 153 (DEC)  

   lora_SetFreq(108, 113, 153);  //433.775 MHz                                                    // Set the LoRa frequency, 434.400 Mhz
// lora_SetFreq(108, 153, 153);  //434.400 MHz                                                  // Set the LoRa frequency, 434.400 Mhz

}

void loop() {


 // calc = 1800;  // 1800sec. = 30 Min.
calc = 900;   // 900sec.  = 15 Min.
//   calc = 600;   // 600sec.  = 10 Min.
//   calc = 300;   // 300sec.  = 5 Min.
//   calc = 60;    //  60sec.  = 1 Min.
//   calc = 30;    //  30sec.  = 30 Sec.

attachInterrupt (digitalPinToInterrupt(interruptPin), isr, RISING);

 

float press = (bmp.readPressure()/10)+380;
float temp = (htu.readTemperature()*1.8);
temp = temp + 32;
float humi = htu.readHumidity();
float lumi = analogRead(36)/100;
float wind2 = windr*0.621;  // wind2 is in mph
float rain_total = 0;
float rain_1h = 0;
String wind_dir = "000";
String rain_st ="000";
String wind_st = "000";

if (cnt_rain>1){
  cnt_rain_cor = cnt_rain_cor + 1;
  cnt_rain = 0;
  }

rain_total = (cnt_rain_cor * 1.181);

adc_v2 = analogRead(39);

switch (adc_v2) // adc read --> convert to win direct
{
case 430 ... 450:
  wind_dir = "045";
  break;
case 1470 ... 1490:
  wind_dir = "000";
  break;
case 0 ... 5:
  wind_dir = "090";
  break;
case 10 ... 30:
  wind_dir = "135";
  break;
case 120 ... 150:
  wind_dir = "180";
  break;
case 840 ... 860:
  wind_dir = "222";
  break;
case 2560 ... 2780:
  wind_dir = "270";
  break;
case 2000 ... 2450:
  wind_dir = "315";
  break;
default:
  break;
}



Outstring = "OK5TVR-6>APRS:!4945.02N/01335.80E_";
Outstring += wind_dir;
Outstring += ("/");
  if(wind2<100) { Outstring += "0"; }
  if(wind2<10)  { Outstring += "0"; }
  wind2 = wind2 * 10;
  wind_st = String(wind2,0);
  wind_st.remove(wind_st.length()-1,1);
  if(wind2==0){ wind_st = "0"; }
Outstring += wind_st;
Outstring += ("g...");
Outstring += ("t");
   if(temp<100) { Outstring += "0"; }
   if(temp<10) { Outstring += "0"; }
Outstring += String(temp,0);
Outstring += ("r");
  if(rain_total<100) { Outstring += "0"; }
  if(rain_total<10)  { Outstring += "0"; }
  rain_total = rain_total * 10;
  rain_st = String(rain_total,0);
  rain_st.remove(rain_st.length()-1,1);
  if(rain_total==0){ rain_st = "0"; }
  Outstring += rain_st;
Outstring += ("p...P...");
Outstring += ("h");
Outstring += String(humi,0);
Outstring += ("b");
   if(press<10000) { Outstring += "0"; }
   Outstring += String(press,0);
Outstring += ("L");
   if(lumi<100) { Outstring += "0"; }
   if(lumi<10) { Outstring += "0"; }
Outstring += String(lumi,0);
Outstring += (" WX-Rokycany by OK5TVR");

if (sekund == calc)
{
  cnt_wind = 0;
  wind_m();
  lora_tx();   
  sekund = 1; 
}

if (cnt_1H == 3600) // time 1H
{
  cnt_rain = 0; 
  cnt_1H = 1;
  rain_total = 0;
  cnt_rain_cor = 0;
}

if (cnt_24H == 86400) // time 24H
{
     
  cnt_24H = 1;
}


    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("OK5TVR-6");
    display.setTextSize(1);
    display.setCursor(5,10);
    display.print(sekund);
    display.setTextSize(1);
    display.setCursor(5,20);
    display.print(String(press,0));
    display.setTextSize(1);
    display.setCursor(50,20);
    display.print(String(lumi,0));
    display.setTextSize(1);
    display.setCursor(5,30);
    display.print(htu.readTemperature());
    display.setTextSize(1);
    display.setCursor(50,30);
    display.print(humi);
    display.setCursor(5,40);
    display.print(rain_st);
    display.setTextSize(1);
    display.setCursor(50,40);
    display.print(wind_st);
    display.setTextSize(1);
    display.setCursor(5,50);
    display.print(wind_dir);
    display.display();  

sekund++;
cnt_1H++;
cnt_24H++;
delay(1000);

// put your main code here, to run repeatedly:
}

void lora_tx()
{
 byte i;
 byte ltemp;
 String text_tx = "";
 text_tx = "tx by ";
 text_tx = text_tx + calc;
 text_tx = text_tx + " s";

 ltemp = Outstring.length();
    lora_SetModem(lora_BW125, lora_SF12, lora_CR4_5, lora_Explicit, lora_LowDoptON);		// Setup the LoRa modem parameters
    lora_PrintModem();                                                                  // Print the modem parameters
    lora_TXStart = 0;
    lora_TXEnd = 0;
    for (i = 0; i <= ltemp; i++)
    {
    lora_TXBUFF[i] = Outstring.charAt(i);
    }
    i--;
    lora_TXEnd = i;    
    digitalWrite(PLED1, HIGH);  // LED ON on during sending
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(0,0);
    display.print("ON AIR");
    display.setTextSize(1);
    display.setCursor(10,30);
    display.print(text_tx);
    display.display();  

    lora_Send(lora_TXStart, lora_TXEnd, 60, 255, 1, 10, 17);	
    digitalWrite(PLED1, LOW);   // LED OFF after sending
    lora_TXBuffPrint(0);
    Outstring = "";
}
void wind_m()
{
attachInterrupt (digitalPinToInterrupt(interruptPinW), wind, FALLING);
delay(3000); //Time for measure counts
detachInterrupt(digitalPinToInterrupt(interruptPinW));
windr = ((float)cnt_wind / (float)3 * 2.4) / 2; //Convert counts & time to km/h
}




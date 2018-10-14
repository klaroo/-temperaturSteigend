/*********************************************************************


*********************************************************************/

#include <math.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 0
Adafruit_SSD1306 display(OLED_RESET);

//#define XPOS 0
//#define YPOS 1
//#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

unsigned int Rs = 9510;
double Vcc = 3300;
int maxAnalogWert = 1024;
int analogWert = 0;
int V_NTC = 0;
int R_NTC = 0;
float temp = 0.0;
float tempLow = 0.0;
boolean alarm = false;
double korrekturFaktor = 0.963;
int onboardLED = D4;
boolean input1 = false;
boolean input2 = false;
boolean istAmMessen = false;

void setup()   {
  Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  // init done

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1);

  pinMode(D0, OUTPUT);
  pinMode(D1, INPUT_PULLUP);
  //pinMode(D2, OUTPUT); used by Display
  pinMode(D3, INPUT_PULLUP);
  pinMode(onboardLED, OUTPUT); // Onboard LED LOW=EIN HIGH=AUS
  pinMode(D5, OUTPUT); // istAmMessen
  pinMode(D6, OUTPUT); // Alarm
  //pinMode(D7, OUTPUT);
  //pinMode(D8, OUTPUT);
  
  digitalWrite(onboardLED,HIGH);
}

void loop() {
  analogWert = analogReadStable();
  V_NTC = Vcc * analogWert / maxAnalogWert * korrekturFaktor;
  R_NTC = Rs / (Vcc - V_NTC) * V_NTC;
  temp = temperature_NTCB(R_NTC);
  displaySchreiben();
  readDigitalInputs();
  if (not(input1)) {
    istAmMessen = not(istAmMessen);};
  digitalWrite(onboardLED,not(istAmMessen));
  digitalWrite(D5,istAmMessen);
  digitalWrite(D6,alarm);
  if (istAmMessen) {
    if (tempLow == 0.0) {
      tempLow = temp;
      } else {
        if (temp < tempLow) {
          tempLow = temp;} else {
            if ((temp - 0.3) > tempLow){
              alarm = true;}}
         }
    } else {
      tempLow = 0.0;
      alarm = false;}
  display.clearDisplay();

  //digitalWrite(onboardLED,(not(digitalRead(onboardLED))));   Blinker :-)
}

void readDigitalInputs() {
  boolean i1 = input1;
  boolean i2 = input2;
  for(int i = 0; i < 50; i++) {
    if (digitalRead(D1) != i1) {
      input1 = not(i1);};
    if (digitalRead(D3) != i2) {
      input2 = not(i2);};
    delay(10);
    }
  }

void displaySchreiben () {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  
  display.setTextSize(1);
  display.println("A = " + (String)analogWert);
  display.println("V = " + (String)V_NTC);
  display.println("R = " + (String)R_NTC);
  display.println();
  //display.setTextSize(2);
  display.println("T = " + (String)temp);
  if (istAmMessen) {
    String str = " " + (String)tempLow;
    if (alarm) {str = str + " ALM";}
    display.println(str);
  }
  display.display();  
  }

int analogReadStable() {
  int val = 0;
  for(int i = 0; i < 20; i++) {
    val += analogRead(A0);
    delay(1);
  }
  return val / 20;
}

// NTC temperature calculation by "jurs" for German Arduino forum
#define ABSZERO 273.15
float temperature_NTCB(int R_NTC)
{
 float T0 = 25.0;
 float R0 = 9660;
 int B = 3700;
 T0+=ABSZERO;  // umwandeln Celsius in absolute Temperatur
 return T0 * B / (B + T0 * log(R_NTC / R0))-ABSZERO;
}

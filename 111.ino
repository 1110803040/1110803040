// REQUIRES the following Arduino libraries:
// - Lien vidéo: https://youtu.be/rUuQf8t5juc
// - Adafruit_NeoPixel: https://github.com/adafruit/Adafruit_NeoPixel
// - ArduinoFFT Library: https://github.com/kosme/arduinoFFT
// - Adafruit_GFX Library: https://github.com/adafruit/Adafruit-GFX-Library
// - Getting Started ESP32 / ESP32S : https://www.youtube.com/watch?v=9b0Txt-yF7E
// Find All "Great Projects" Videos : https://www.youtube.com/c/GreatProjects


#include "SPIFFS.h"
#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <arduinoFFT.h>

#define PIN        15 
#define NUMPIXELS 128 
#define EEPROM_SIZE 7

arduinoFFT FFT = arduinoFFT();
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
WebServer server(80);
GFXcanvas1 canvasF(8, 8);

char ssid[30] = "";
char pass[30] = "";
const char* Apssid = "NeoPixel";
const char* Appassword = "00000000";
  IPAddress ip(192, 168, 1, 141);
  IPAddress dns(192, 168, 1, 1);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress gatewayap(192,168,4,1);
  IPAddress subnet(255, 255, 255, 0);

File myfile;
long savednum = 0 ,passnum = 0; 
uint32_t lastTime, lastdoT, lastblack; 
String text;
#define SAMPLES 1024 
double vReal[SAMPLES];
double vImag[SAMPLES];
int Exval[8] = {0,0,0,0,0,0,0,0};  
int Exdot[8] = {0,0,0,0,0,0,0,0};
int bandValues[8] = {0,0,0,0,0,0,0,0};
int vR, yh;
int sR1, sR2, sR3, sR4;
int sp0, sp1, sp2, sp3, sp4, sp5, sp6, sp7;
long     hueShift;
long     value;
int      dx1, dx2, dx3, dx4, dy1, dy2, dy3, dy4, sx1, sx2, sx3, sx4;
int      r, g, b;
uint8_t  sat, val, lo;
float rs1, rs2, rs3, rs4,
      cx1, cx2, cx3, cx4,
      cy1, cy2, cy3, cy4;
float  ag1, ag2, ag3, ag4;
static const int8_t PROGMEM sinetab[256] = {
     0,   2,   5,   8,  11,  15,  18,  21, 24,  27,  30,  33,  36,  39,  42,  45,
    48,  51,  54,  56,  59,  62,  65,  67, 70,  72,  75,  77,  80,  82,  85,  87,
    89,  91,  93,  96,  98, 100, 101, 103, 105, 107, 108, 110, 111, 113, 114, 116,
   117, 118, 119, 120, 121, 122, 123, 123, 124, 125, 125, 126, 126, 126, 126, 126,
   127, 126, 126, 126, 126, 126, 125, 125, 124, 123, 123, 122, 121, 120, 119, 118,
   117, 116, 114, 113, 111, 110, 108, 107, 105, 103, 101, 100,  98,  96,  93,  91,
    89,  87,  85,  82,  80,  77,  75,  72, 70,  67,  65,  62,  59,  56,  54,  51,
    48,  45,  42,  39,  36,  33,  30,  27, 24,  21,  18,  15,  11,   8,   5,   2,
     0,  -3,  -6,  -9, -12, -16, -19, -22, -25, -28, -31, -34, -37, -40, -43, -46,
   -49, -52, -55, -57, -60, -63, -66, -68, -71, -73, -76, -78, -81, -83, -86, -88,
   -90, -92, -94, -97, -99,-101,-102,-104, -106,-108,-109,-111,-112,-114,-115,-117,
  -118,-119,-120,-121,-122,-123,-124,-124, -125,-126,-126,-127,-127,-127,-127,-127,
  -128,-127,-127,-127,-127,-127,-126,-126, -125,-124,-124,-123,-122,-121,-120,-119,
  -118,-117,-115,-114,-112,-111,-109,-108, -106,-104,-102,-101, -99, -97, -94, -92,
   -90, -88, -86, -83, -81, -78, -76, -73, -71, -68, -66, -63, -60, -57, -55, -52,
   -49, -46, -43, -40, -37, -34, -31, -28, -25, -22, -19, -16, -12,  -9,  -6,  -3
};

void(* resetFunc) (void) = 0;//declare reset function at address 0

void getWifi()
{  
  if (WiFi.status() == WL_CONNECTED) { 
    WiFi.disconnect();
    }
  WiFi.config(ip, dns, gateway, subnet);
  WiFi.begin(ssid, pass);
  int xc = 0;
  while (WiFi.status() != WL_CONNECTED && xc < 10) {
    delay(300);
    xc++;
  }
  if (WiFi.status() != WL_CONNECTED) { 
    WiFi.softAPConfig(ip, gatewayap, subnet);
    WiFi.softAP(Apssid, Appassword);
    } else { 
    WiFi.softAPdisconnect(true);
    }
}
void Colorplat(int cpla) {
  switch (cpla) { 
  case 0:
  rs1 = 7.1, rs2 = 9.3, rs3 = 6.2, rs4 = 3.1;
  cx1 = 3.1, cy1 = 4.9;
  cx2 = 5.8, cy2 = 12.5;
  cx3 = 2.8, cy3 = 8.3;
  cx4 = 3.1, cy4 = -0.5;
  sp4 = 1, sp5 = 2;
    break;
  case 1:
  rs1 = 11.1, rs2 = 9.3, rs3 = 7.2, rs4 = 5.1;
  cx1 = 7.8, cy1 = 2.5;
  cx2 = 4.1, cy2 = 1.9;
  cx3 = 2.8, cy3 = 2.3;
  cx4 = 3.1, cy4 = -2.5;
  sp4 = 2, sp5 = 3;
    break;
  case 2:
  rs1 = 26.1, rs2 = 26.3, rs3 = 27.2, rs4 = 57.1;
  cx1 = 7.8, cy1 = 2.5;
  cx2 = 4.1, cy2 = 11.9;
  cx3 = 2.8, cy3 = 18.3;
  cx4 = 3.1, cy4 = -12.5;
  sp4 = 3, sp5 = 3;
    break;
  case 3:
  rs1 = 16.3, rs2 = 23.0, rs3 = 40.8, rs4 = 44.2;
  cx1 = 896.0, cy1 = 89.8;
  cx2 = 42.4, cy2 = 80.2;
  cx3 = 48.4, cy3 = 48.0;
  cx4 = -296.0, cy4 = -11.8;
  sp4 = 5, sp5 = 4;
    break;
  case 4:
  rs1 = 7.1, rs2 = 9.3, rs3 = 6.2, rs4 = 3.1;
  cx1 = 4.1, cy1 = 1.9;
  cx2 = 2.8, cy2 = 8.3;
  cx3 = 7.8, cy3 = 42.5;
  cx4 = 3.1, cy4 = -2.5;
  sp4 = 3, sp5 = 2;
    break;
  case 5:
  rs1 = 7.1, rs2 = 18.3, rs3 = 13.2, rs4 = 7.1;
  cx1 = 3.1, cy1 = 4.9;
  cx2 = 5.8, cy2 = 12.5;
  cx3 = 2.8, cy3 = 8.3;
  cx4 = 3.1, cy4 = -7.5;
  sp4 = 1, sp5 = 3;
    break;
  case 6:
  rs1 = 16.3, rs2 = 23.0, rs3 = 40.8, rs4 = 44.2;
  cx1 = 96.0, cy1 = 89.8;
  cx2 = 42.4, cy2 = 80.2;
  cx3 = 48.4, cy3 = 48.0;
  cx4 = -26.0, cy4 = -11.8;
  sp4 = 5, sp5 = 4;
    break;
  case 7:
  rs1 = 56.1, rs2 = 48.3, rs3 = 53.2, rs4 = 57.1;
  cx1 = 4.1, cy1 = 1.9;
  cx2 = 7.8, cy2 = 42.5;
  cx3 = 2.8, cy3 = 8.3;
  cx4 = 3.1, cy4 = -2.5;
  sp4 = 8, sp5 = 4;
    break;
  case 8:
  rs1 = 16.3, rs2 = 23.0, rs3 = 40.8, rs4 = 44.2;
  cx1 = 896.0, cy1 = 89.8;
  cx2 = 42.4, cy2 = 80.2;
  cx3 = 748.4, cy3 = 448.0;
  cx4 = -296.0, cy4 = 111.8;
  sp4 = 7, sp5 = 9;
    break;
  case 9:
  rs1 = 36.1, rs2 = 92.3, rs3 = 69.2, rs4 = 60.1;
  cx1 = 512.8, cy1 = 278.2;
  cx2 = 42.4, cy2 = 80.2;
  cx3 = 748.4, cy3 = 448.0;
  cx4 = 96.0, cy4 = -289.8;
  sp4 = 7, sp5 = 9;
    break;
  case 10:
  rs1 = 49.0, rs2 = 69.0, rs3 = 122.4, rs4 = 132.6;
  cx1 = 40.1, cy1 = 83.1;
  cx2 = 44.4, cy2 = 87.2;
  cx3 = 43.4, cy3 = 60.2;
  cx4 = 45.2, cy4 = 61.2;
  sp4 = 5, sp5 = 6;
    break;
  default:
  rs1 = 49.0, rs2 = 69.0, rs3 = 122.4, rs4 = 132.6;
  cx1 = 40.1, cy1 = 83.1;
  cx2 = 44.4, cy2 = 87.2;
  cx3 = 43.4, cy3 = 60.2;
  cx4 = 45.2, cy4 = 61.2;
  sp4 = 5, sp5 = 6;
    break;
  }
}
void getColor(long hue) {
  // Hue
  hue %= 1791; // -1791 to +1791
  if (hue < 0)
    hue += 1791;      //     0 to +1791
  lo = hue & 255;     // Low byte  = primary/secondary color mix
  switch (hue >> 8) { // High byte = sextant of colorwheel
  case 0:
    r = 255;
    g = 255 - lo;
    b = 255 - lo;
    break; // W to R
  case 1:
    r = 255;
    g = lo;
    b = 0;
    break; // R to Y
  case 2:
    r = 255 - lo;
    g = 255;
    b = 0;
    break; // Y to G
  case 3:
    r = 0;
    g = 255;
    b = lo;
    break; // G to C
  case 4:
    r = 0;
    g = 255 - lo;
    b = 255;
    break; // C to B
  case 5:
    r = lo;
    g = 0;
    b = 255;
    break; // B to M
  default:
    r = 255;
    g = lo;
    b = 255;
    break; // M to W
  }
}
void getblat() {
  sx1 = (int)(cos(ag1) * rs1 + cx1);
  sx2 = (int)(cos(ag2) * rs2 + cx2);
  sx3 = (int)(cos(ag3) * rs3 + cx3);
  sx4 = (int)(cos(ag4) * rs4 + cx4);
  dy1 = (int)(sin(ag1) * rs1 + cy1);
  dy2 = (int)(sin(ag2) * rs2 + cy2);
  dy3 = (int)(sin(ag3) * rs3 + cy3);
  dy4 = (int)(sin(ag4) * rs4 + cy4);
  for(int x=0; x<8; x++) {
  int xl = (7-x)*8, xr = (8+x)*8; 
  dx1 = sx1; dx2 = sx2; dx3 = sx3; dx4 = sx4;
  for(int y=0; y<8; y++) {
  int c0 = canvasF.getPixel(x, y);
  if (c0 != 0) {
      value = hueShift
        + (int8_t)pgm_read_byte(sinetab + (uint8_t)((dx1 * dx2 - dy1 * dy2) >> sp4))
        + (int8_t)pgm_read_byte(sinetab + (uint8_t)((dx3 * dx4 + dy3 * dy4) >> sp4))
        + (int8_t)pgm_read_byte(sinetab + (uint8_t)((dx1 * dx2 - dy1 * dy2) >> sp5))
        + (int8_t)pgm_read_byte(sinetab + (uint8_t)((dx3 * dx4 + dy3 * dy4) >> sp5));
  getColor(value*3);  
  pixels.setPixelColor(xl+y, pixels.Color(r,g,b));
  pixels.setPixelColor(xr+y, pixels.Color(r,g,b));
    } else {  
  pixels.setPixelColor(xl+y, pixels.Color(0,0,0));
  pixels.setPixelColor(xr+y, pixels.Color(0,0,0));
  }
      dx1--; dx2--; dx3--; dx4--;
    }
    dy1--; dy2--; dy3--; dy4--;
  }
    pixels.show();
}
void getdoT() {
  for(int x=0; x<8; x++) {
      yh = Exdot[x]; 
      if (yh < 8) { 
      Exdot[x]+=1; 
  }
  } 
}
void getblack() {
  for(int x=0; x<8; x++) {
      yh = Exval[x]; 
      if (yh < 7) { 
      Exval[x]+=1; 
  }
  }    
}
void displayBand(int ban, int vmic){
      if (Exdot[ban] > vmic) { Exdot[ban] = vmic; }
      if (Exval[ban] > vmic) { Exval[ban] = vmic; }
      canvasF.fillRect(ban, 0, 1, Exval[ban], 0x0000);
      canvasF.fillRect(ban, Exdot[ban], 1, 1, 0xFFFF);
      canvasF.fillRect(ban, vmic, 1, 8-vmic, 0xFFFF);
      if (vmic < 7) {
      if (ban<=1) { sR1 += vmic; }
      else if (ban<=3) { sR2 += vmic; }
      else if (ban<=5) { sR3 += vmic; }
      else if (ban<=7) { sR4 += vmic; }
        }
}
void getdata(){
  for (int i = 0; i < SAMPLES; i++) {
    vReal[i] = analogRead(34);
    vImag[i] = 0;
  }
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
  sR1 = 0; sR2 = 0; sR3 = 0; sR4 = 0;
  hueShift = 0;
  for (int i = 2; i < SAMPLES/2; i++){ 
      if (i<=2 )              bandValues[7]  += (int)vReal[i]; // 125Hz
      if (i >2   && i<=4   )  bandValues[6]  += (int)vReal[i]; // 250Hz
      if (i >4   && i<=7   )  bandValues[5]  += (int)vReal[i]; // 500Hz
      if (i >7   && i<=15  )  bandValues[4]  += (int)vReal[i]; // 1000Hz
      if (i >15  && i<=40  )  bandValues[3]  += (int)vReal[i]; // 2000Hz
      if (i >40  && i<=70  )  bandValues[2]  += (int)vReal[i]; // 4000Hz
      if (i >70  && i<=95  )  bandValues[1]  += (int)vReal[i]; // 8000Hz
      if (i >95  && i<=125 )  bandValues[0]  += (int)vReal[i]; // 16000Hz
      if (i >125           )  hueShift       += (int)vReal[i]; 
    } 
  for (byte band = 0; band < 8; band++) {
    bandValues[band] /= 100;
    vR = map(bandValues[band], 0, sp0, 8, 0);
    if (vR < 0) { vR = 0; }
    displayBand(band,vR);
    bandValues[band]=0;
  }
  ag1 = sR1*0.011;
  ag2 = -sR2*0.033;
}
  void handleRoot() {
  File file = SPIFFS.open("/setting.html");
  if (file) {
    size_t sent = server.streamFile(file, "text/html"); //Send web page
  }
    file.close();
}
void handleRestesp() {  
    handleRoot();
    delay(4000);     
    resetFunc();
}
void handleBright() {
    handleRoot();
    sp7 = server.arg("Bright").toInt();
    EEPROM.write(6, sp7);
    EEPROM.commit();
    pixels.setBrightness(sp7);
  }
void handleSpe0() {
    handleRoot();
    sp0 = server.arg("Speed0").toInt();
    EEPROM.write(1, sp0);
    EEPROM.commit();
    sp0 = (sp0*100)+1600;
  }
void handleSpe1() {
    handleRoot();
    sp1 = server.arg("Speed1").toInt();
    EEPROM.write(2, sp1);
    EEPROM.commit();
  }
void handleSpe2() { 
    handleRoot();
    sp2 = server.arg("Speed2").toInt();
    EEPROM.write(3, sp2);
    EEPROM.commit();
  }
void handleSpe3() { 
    handleRoot();
    sp3 = server.arg("Speed3").toInt();
    EEPROM.write(4, sp3);
    EEPROM.commit();
    sp3 *= 4;
  }
void handleSpe4() { 
    handleRoot();
    sp4 = server.arg("Speed4").toInt();
  }
void handleSpe5() { 
    handleRoot();
    sp5 = server.arg("Speed5").toInt();
  }
void handleSpe6() { 
    handleRoot();
    sp6 = server.arg("Speed6").toInt();
    EEPROM.write(5, sp6);
    EEPROM.commit();
    Colorplat(sp6);
  }
void handleWifi() { 
  handleRoot();
  text = server.arg("usname");
  myfile = SPIFFS.open("/Ssid.txt", FILE_WRITE);
    if(myfile){ 
      myfile.print(text); 
    }
    myfile.close();
  int len = text.length() + 1; 
  text.toCharArray(ssid, len);
  text = server.arg("pssw");
  myfile = SPIFFS.open("/Password.txt", FILE_WRITE);
    if(myfile){ 
      myfile.print(text);
    }
    myfile.close();
  len = text.length() + 1;
  text.toCharArray(pass, len); 
  getWifi();
}
void getssid() { 
myfile = SPIFFS.open("/Ssid.txt");
 if(!myfile){
   return;
 }
int nb = 0;
while (myfile.available()) {
  savednum = myfile.read();
  ssid[nb] = (char) savednum;
  nb += 1;
}
myfile.close();
}
void getpass() { 
myfile = SPIFFS.open("/Password.txt");
 if(!myfile){
   return;
 }
int nb = 0;
while (myfile.available()) {
  savednum = myfile.read();
  pass[nb] = (char) savednum;
  nb += 1;
}
myfile.close();
}
void getstting() { 
    sp0 = EEPROM.read(1);
    sp1 = EEPROM.read(2);
    sp2 = EEPROM.read(3);
    sp3 = EEPROM.read(4);
    sp6 = EEPROM.read(5);
    sp7 = EEPROM.read(6);
    sp0 = (sp0*100)+1600;
    sp3 *= 4;
}
void setup() {
  SPIFFS.begin(true);
  EEPROM.begin(EEPROM_SIZE);
  pixels.begin(); 
  canvasF.setTextWrap(false);
  getssid();
  getpass();
  getWifi();
  server.on("/", handleRoot); 
  server.on("/MesBright", handleBright);
  server.on("/MesSpeed0", handleSpe0);
  server.on("/MesSpeed1", handleSpe1); 
  server.on("/MesSpeed2", handleSpe2); 
  server.on("/MesSpeed3", handleSpe3); 
  server.on("/MesSpeed4", handleSpe4); 
  server.on("/MesSpeed5", handleSpe5);
  server.on("/MesSpeed6", handleSpe6); 
  server.on("/Mywifi", handleWifi); 
  server.on("/restesp", handleRestesp);
  server.begin(); 
  getstting();
  Colorplat(sp6); 
  pixels.setBrightness(sp7);
  pixels.show(); 
  delay(2000);
}
void loop() {
  server.handleClient();
   if(millis() - lastTime >= sp1) {
      getdata();
      getblat();
      lastTime = millis();
    }
   if (millis() - lastblack >= sp2) {
      getblack();
      lastblack = millis();
    }
   if(millis() - lastdoT >= sp3) {
      getdoT();
      lastdoT = millis();
    }
}

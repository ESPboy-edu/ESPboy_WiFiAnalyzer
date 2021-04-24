/*
 WiFi Analyzer for ESPboy project www.espboy.com by RomanS
 Revised from ESP8266WiFi WiFiScan example.
 */

#include "ESPboyInit.h"
#include "U8g2_for_TFT_eSPI.h"

#define WIDTH 128
#define HEIGHT 128
#define GRAPH_BASELINE (HEIGHT - 18)
#define GRAPH_HEIGHT (HEIGHT - 52)
#define CHANNEL_WIDTH (WIDTH / 15)

// RSSI RANGE
#define RSSI_CEILING -40
#define RSSI_FLOOR -100
#define NEAR_CHANNEL_RSSI_ALLOW -70

ESPboyInit myESPboy;
U8g2_for_TFT_eSPI u8f;


// Channel color mapping from channel 1 to 14
uint16_t channel_color[] = {
  TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_MAGENTA,
  TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_CYAN, TFT_MAGENTA,
  TFT_RED, TFT_ORANGE
};

uint8_t scan_count = 0;


void setup(){
  //Init ESPboy
  myESPboy.begin(((String)F("WiFi analyzer")).c_str());
  
//u8f init
  u8f.begin(myESPboy.tft);
  u8f.setFontMode(1);                 // use u8g2 none transparent mode
  u8f.setBackgroundColor(TFT_BLACK);
  u8f.setFontDirection(0);            // left to right
  u8f.setFont(u8g2_font_4x6_t_cyrillic); 

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(500);
}


void loop() {
  uint8_t ap_count[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int32_t max_rssi[] = {-100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100, -100};
  String toPrint;

  tone(SOUNDPIN, 200, 100);
  u8f.setForegroundColor(TFT_WHITE);
  u8f.drawStr(30, 28, "Scanning WiFi...");
  int n = WiFi.scanNetworks();
  
  myESPboy.tft.fillScreen(TFT_BLACK);
  
  if (n == 0) {
    u8f.setForegroundColor(TFT_RED);
    u8f.drawStr(24, 35, "No networks found");
  } 
  else {

  toPrint = "WiFi found: " + (String)n;
  u8f.setForegroundColor(TFT_ORANGE);
  u8f.drawStr(0, 8, toPrint.c_str());


  // draw graph base axle
  myESPboy.tft.drawFastHLine(0, GRAPH_BASELINE, 320, TFT_WHITE);
  for (int i = 1; i <= 14; i++) {
    u8f.setForegroundColor(channel_color[i - 1]);
    u8f.drawStr((i * CHANNEL_WIDTH) - ((i < 10)?1:3), GRAPH_BASELINE + 8, ((String)i).c_str());
  }

   // plot found WiFi info
    for (int i = 0; i < n; i++) {
      int32_t channel = WiFi.channel(i);
      int32_t rssi = WiFi.RSSI(i);
      uint16_t color = channel_color[channel - 1];
      int height = constrain(map(rssi, RSSI_FLOOR, RSSI_CEILING, 1, GRAPH_HEIGHT), 1, GRAPH_HEIGHT);

      // channel stat
      ap_count[channel - 1]++;
      if (rssi > max_rssi[channel - 1]) {
        max_rssi[channel - 1] = rssi;
      }

      myESPboy.tft.drawLine(channel * CHANNEL_WIDTH, GRAPH_BASELINE - height, (channel - 1) * CHANNEL_WIDTH, GRAPH_BASELINE + 1, color);
      myESPboy.tft.drawLine(channel * CHANNEL_WIDTH, GRAPH_BASELINE - height, (channel + 1) * CHANNEL_WIDTH, GRAPH_BASELINE + 1, color);

      // Print SSID, signal strengh and if not encrypted
      u8f.setForegroundColor(color);
      u8f.drawStr((channel - 1) * CHANNEL_WIDTH, GRAPH_BASELINE - 10 - height, WiFi.SSID(i).c_str());
      delay(200);
    } 

  
  
  // print WiFi stat
  toPrint = "\nRecom.ch: ";
  bool listed_first_channel = false;
  for (int i = 1; i <= 11; i++) { // channels 12-14 may not available
    if ((i == 1) || (max_rssi[i - 2] < NEAR_CHANNEL_RSSI_ALLOW)) { // check previous channel signal strengh
      if ((i == sizeof(channel_color)) || (max_rssi[i] < NEAR_CHANNEL_RSSI_ALLOW)) { // check next channel signal strengh
        if (ap_count[i - 1] == 0) { // check no AP exists in same channel
          if (!listed_first_channel) {
            listed_first_channel = true;
          } else {
            toPrint+="/";
          }
          toPrint+=(String)i;
        }
      }
    }
  }
  u8f.setForegroundColor(TFT_GREEN);
  u8f.drawStr(0, 16, toPrint.c_str());

  // draw graph base axle
  for (int i = 1; i <= 14; i++) {
    u8f.setForegroundColor(channel_color[i - 1]);
    if (ap_count[i - 1] > 0) {
      String toPrint = "." + (String)ap_count[i - 1] + ".";
      u8f.drawStr((i * CHANNEL_WIDTH) - ((ap_count[i - 1] < 10)?5:10), GRAPH_BASELINE + 15, toPrint.c_str());
    }
  }
}

  // Wait a bit before scanning again
  delay(3000);
}

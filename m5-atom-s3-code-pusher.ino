#include "USB.h"
#include "USBHIDKeyboard.h"
#include "WiFi.h"
#include "esp_wps.h"
#include "M5GFX.h"
#include <HTTPClient.h>

// DEF Keyboard
USBHIDKeyboard Keyboard;
const int buttonPin = 41;
int previousButtonState = HIGH;

// DEF Display
M5GFX display;
M5Canvas c_cons(&display);

// DEF WIFI
static esp_wps_config_t config;
#define ESP_WPS_MODE      WPS_TYPE_PBC
#define ESP_MANUFACTURER  "RAWSEQ"
#define ESP_MODEL_NUMBER  "CPUSHER"
#define ESP_MODEL_NAME    "CODE PUSHER IOT"
#define ESP_DEVICE_NAME   "CPUSHER STATION"

// DEF Function
void updateConsole(void *arg);
void WiFiEvent(WiFiEvent_t event, arduino_event_info_t info);
void wpsInitConfig();
void wpsStart();
void wpsStop();

// Main
void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  Keyboard.begin();
  USB.begin();

  display.begin();
  display.setRotation(3);
  c_cons.setColorDepth(1);
  c_cons.createSprite(128, 128);
  c_cons.setTextSize(1);
  c_cons.setTextScroll(true);
  c_cons.printf("CODE PUSHER OK\n");
  xTaskCreatePinnedToCore(updateConsole, "updateConsole", 4096, NULL, 1, NULL, 0);

  WiFi.begin();
  c_cons.printf("WIFI Connecting.");

  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    c_cons.printf(".");
    count++;
    if (count == 10) {
      WiFi.onEvent(WiFiEvent);
      WiFi.mode(WIFI_MODE_STA);
      c_cons.printf("NG\nWPS Starting.");
      wpsInitConfig();
      wpsStart();
    }
  }
  c_cons.printf("Connected\n");

}

// Loop
void loop() {
  int buttonState = digitalRead(buttonPin);
  if ((buttonState != previousButtonState) && (buttonState == LOW)) {
    HTTPClient hc;
    hc.begin("https://url_code_api/");
    hc.GET();
    String ccode = hc.getString();
    hc.end();
    c_cons.printf("Input CODE:");
    c_cons.printf("%s", ccode);
    c_cons.printf("\n");
    Keyboard.print(ccode);

  }
  previousButtonState = buttonState;
  delay(1);
}

// Sub
void updateConsole(void *arg)
{
  while (1)
  {
    c_cons.pushSprite(0, 0);
    delay(10);
  }
}

void WiFiEvent(WiFiEvent_t event, arduino_event_info_t info){
  switch(event){
    case ARDUINO_EVENT_WIFI_STA_START:
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      //c_cons.printf("Connected to : %s\n", String(WiFi.SSID()));
      //c_cons.printf("Got IP: %s\n", String(WiFi.localIP()));
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      WiFi.reconnect();
      break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
      wpsStop();
      delay(10);
      WiFi.begin();
      break;
    case ARDUINO_EVENT_WPS_ER_FAILED:
      wpsStop();
      wpsStart();
      break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
      wpsStop();
      wpsStart();
      break;
    case ARDUINO_EVENT_WPS_ER_PIN:
      break;
    default:
      break;
  }
}

void wpsInitConfig(){
  config.wps_type = ESP_WPS_MODE;
  strcpy(config.factory_info.manufacturer, ESP_MANUFACTURER);
  strcpy(config.factory_info.model_number, ESP_MODEL_NUMBER);
  strcpy(config.factory_info.model_name, ESP_MODEL_NAME);
  strcpy(config.factory_info.device_name, ESP_DEVICE_NAME);
}

void wpsStart(){
    if(esp_wifi_wps_enable(&config)){
    	c_cons.printf("WPS Enable Failed\n");
    } else if(esp_wifi_wps_start(0)){
    	c_cons.printf("WPS Start Failed\n");
    }
}

void wpsStop(){
    if(esp_wifi_wps_disable()){
    	// Serial.println("WPS Disable Failed");
    }
}


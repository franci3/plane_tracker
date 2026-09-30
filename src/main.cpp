#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>

#include "wifi_connection.h"
#include "../lib/OpenSkyNetwork/openskynetwork.h"
#include "../lib/secrets.h"

constexpr u_int8_t TFT_DC{D1};
constexpr u_int8_t TFT_RST{D2};
constexpr u_int8_t TFT_CS{D8};
constexpr u_int8_t BUTTON_D6{D6};
constexpr u_int kDelayMS{1000};
constexpr float kLongitude{LONGITUDE};
constexpr float kLatitude{LATITUDE};
static auto tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
static BearSSL::WiFiClientSecure client;
static HTTPClient http;
static void clearDisplay();
static void connectWifi();
static void showCurrentPlanes();
static void readButtonState();
static void updatePlanes();
static String token{};
static std::vector<openskynetwork::Plane> planes;
static bool buttonClicked{};

void setup()
{
    Serial.begin(9600);
    tft.init(240, 280, SPI_MODE0);

    tft.setRotation(3);
    clearDisplay();
    connectWifi();
    delay(kDelayMS);
    clearDisplay();
    client.setInsecure();
    token = openskynetwork::getBearerAuthToken(client, http);
    tft.setTextWrap(false);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_BLUE);

    updatePlanes();
}

void loop()
{
    readButtonState();
    tft.setCursor(0, 130);
    clearDisplay();
    showCurrentPlanes();
    delay(kDelayMS);
}

void clearDisplay()
{
    tft.fillScreen(ST77XX_BLACK);
}

void showCurrentPlanes()
{
    if (planes.empty())
    {
        tft.write("No planes in area");
        return;
    }
    const auto& plane = planes[0];
    tft.print(plane.callSign);
}

void connectWifi()
{
    tft.setCursor(0, 20);
    tft.println("Setting up Wifi connection");
    const char* ssid = SSID;
    const char* password = PASSWORD;
    const auto result = wifi_connection::connnectWifi(ssid, password);
    tft.println(result);
}

void readButtonState()
{
    if (const int rawValue = analogRead(A0); rawValue > 512 && !buttonClicked)
    {
        buttonClicked = true;
        updatePlanes();
    }
    else
    {
        buttonClicked = false;
    }
    Serial.println(buttonClicked);
}

void updatePlanes()
{
    planes = openskynetwork::getCurrentPlanes(
        kLatitude, kLongitude, token, client, http);
    Serial.println("Planes currently active: " + String(planes.size()));
}

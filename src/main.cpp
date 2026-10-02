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
constexpr u_int kDelayMS{1000};
constexpr u_int kUpdateIntervalMS{600000};
constexpr float kLongitude{LONGITUDE};
constexpr float kLatitude{LATITUDE};
static auto tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
static BearSSL::WiFiClientSecure client;
static HTTPClient http;
static void clearDisplay();
static void connectWifi();
static void showCurrentPlanes();
static void updatePlanes();
static void displayPlanes(const openskynetwork::Plane& plane, u_int index, size_t count);
static String token{};
static std::vector<openskynetwork::Plane> planes;
static unsigned long elapsedTime;

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
    elapsedTime = millis();
}

void loop()
{
    if (millis() - elapsedTime < kUpdateIntervalMS)
    {
        tft.setCursor(0, 130);
        clearDisplay();
        showCurrentPlanes();
        delay(kDelayMS);
    }
    else
    {
        updatePlanes();
        elapsedTime = millis();
    }
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
    u_int index{};
    for (const auto& plane : planes)
    {
        displayPlanes(plane, index++, planes.size());
        delay(kDelayMS * 3);
        clearDisplay();
    }
}

void displayPlanes(const openskynetwork::Plane& plane, const u_int index, const size_t count)
{
    tft.setCursor(0, 30);
    tft.println(String(index + 1) + "/" + count);
    tft.println(plane.callSign);
    tft.println(plane.originCountry);
    tft.println(String(plane.distanceToSelf) + " km");
    tft.println(String(plane.geoAltitude) + " m ue. NN");
    tft.println(String(plane.velocity) + " m/s");
    tft.println("Category " + String(plane.category));
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

void updatePlanes()
{
    planes = openskynetwork::getCurrentPlanes(
        kLatitude, kLongitude, token, client, http);
    Serial.println("Planes currently active: " + String(planes.size()));
}

#include "wifi_connection.h"
#include <ESP8266WiFi.h>

namespace wifi_connection
{
    String connnectWifi(const char* ssid, const char* password)
    {
        WiFi.begin(ssid, password);
        int count{};
        while (WiFi.status() != WL_CONNECTED && count < 10)
        {
            delay(500);
            count++;
        }
        if (WiFi.status() != WL_CONNECTED)
        {
            return "WIFI not connected after timeout";
        }
        const auto localIP = WiFi.localIP();
        return localIP.toString();
    }
}

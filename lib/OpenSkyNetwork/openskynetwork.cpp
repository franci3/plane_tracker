#include "openskynetwork.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <vector>
#include "../secrets.h"

namespace openskynetwork
{
    String getBearerAuthToken(WiFiClient& client, HTTPClient& http)
    {
        constexpr auto bearerUrl =
            "https://auth.opensky-network.org/auth/realms/opensky-network/protocol/openid-connect/token";

        http.begin(client, bearerUrl);

        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        char buf[120];
        snprintf(buf, sizeof(buf), "grant_type=client_credentials&client_id=%s&client_secret=%s", CLIENT_ID,
                 CLIENT_SECRET);
        if (const int httpResponseCode = http.POST(buf); httpResponseCode != 200)
        {
            return "Failed to get Bearer Token: " + String(httpResponseCode) + " -> " + http.getString();
        }

        auto payload = http.getString();
        http.end();

        JsonDocument doc;
        deserializeJson(doc, payload);

        return doc["access_token"];
    }

    std::vector<Plane> getCurrentPlanes(const float latitude, const float longitude,
                                        const String& bearerToken, WiFiClient& client,
                                        HTTPClient& http)
    {
        const auto boundingValues = getBoundingValues(latitude, longitude);

        const auto url = "https://opensky-network.org/api/states/all?lamin=" + String(boundingValues[0]) + "&lomin=" +
            String(boundingValues[1]) + "&lamax=" + String(boundingValues[2]) + "&lomax=" + String(boundingValues[3]);
        http.begin(
            client, url);

        http.addHeader("Authorization", "Bearer " + bearerToken);

        if (const int httpResponseCode = http.GET(); httpResponseCode != 200)
        {
            http.end();
            return {};
        }

        const String payload = http.getString();

        http.end();

        return parsePlanes(payload);
    }

    std::vector<float> getBoundingValues(const float& latitude, const float& longitude)
    {
        constexpr u_int radius = 20;
        constexpr float pi = 3.14159;
        constexpr float earthRadius = 6378.137;
        constexpr float degreesPerKMLat = 0.0089831117;
        constexpr float latDelta = radius * degreesPerKMLat;

        const float latRadians = latitude * pi / 180;

        const float minLatitude = latitude - latDelta;
        const float maxLatitude = latitude + latDelta;

        const float degreesPerKmLon = 1 / (earthRadius * cos(latRadians) * pi / 180);
        const float lonDelta = radius * degreesPerKmLon;

        const float minLongitude = longitude - lonDelta;
        const float maxLongitude = longitude + lonDelta;

        return std::vector{minLatitude, minLongitude, maxLatitude, maxLongitude};
    }


    std::vector<Plane> parsePlanes(const String& payload)
    {
        JsonDocument doc;
        deserializeJson(doc, payload);

        const auto states = doc["states"];

        if (states.isNull())
        {
            return {};
        }

        const auto firstObj = states[0];
        return {Plane(firstObj[1], firstObj[2], firstObj[13], firstObj[7], 1.0, firstObj[9])};
    }
}

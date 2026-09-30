#include "openskynetwork.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <vector>
#include "../secrets.h"

namespace
{
    constexpr float pi = 3.14159;
    constexpr float earthRadius = 6378.137;

    std::vector<float> getBoundingValues(const float latitude, const float longitude)
    {
        constexpr u_int radius = 20;
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

    float calculateDistanceToSelf(const float selfLongitude, const float selfLatitude, const float planeLongitude,
                                  const float planeLatitude, const float altitude)
    {
        // Using the Equirectangular approximation since the distance is usually small enough to ignore earths curvature
        const auto phi1 = radians(selfLatitude);
        const auto phi2 = radians(planeLatitude);
        const auto lambda1 = radians(selfLongitude);
        const auto lambda2 = radians(planeLongitude);
        const auto x = (lambda1 - lambda2) * cos((phi1 + phi2) / 2);
        const auto y = phi2 - phi1;

        const auto distance = earthRadius * sqrt(pow(x, 2) + pow(y, 2));

        // Altitude matters if the plane is in the air
        if (altitude <= 10)
        {
            return static_cast<float>(distance);
        }

        const auto altitudeInKM = altitude / 1000;
        const auto c = pow(distance, 2) + pow(altitudeInKM, 2);
        return static_cast<float>(sqrt(c));
    }


    std::vector<openskynetwork::Plane> parsePlanes(const String& payload, const float latitude, const float longitude)
    {
        JsonDocument doc;
        deserializeJson(doc, payload);

        const JsonArray data = doc["states"];

        if (data.isNull())
        {
            return {};
        }

        std::vector<openskynetwork::Plane> planes{};

        for (const auto plane : data)
        {
            planes.push_back(openskynetwork::Plane(plane[1], plane[2], plane[13], plane[7], calculateDistanceToSelf(
                                                       longitude, latitude, plane[5], plane[6], plane[13]
                                                   ), plane[9], plane[17]));
        }

        return planes;
    }
}

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

        return parsePlanes(payload, latitude, longitude);
    }
}

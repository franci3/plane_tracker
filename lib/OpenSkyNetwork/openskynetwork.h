#pragma once

#include <vector>
#include <WString.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

namespace openskynetwork
{
    struct Plane
    {
        String callSign;
        String originCountry;
        float geoAltitude;
        float baroAltitude;
        float distanceToSelf;
        float velocity;
        u_int category;
    };

    String getBearerAuthToken(WiFiClient& client, HTTPClient& http);
    std::vector<Plane> getCurrentPlanes(float latitude, float longitude,
                                        const String& bearerToken, WiFiClient& client,
                                        HTTPClient& http);
}

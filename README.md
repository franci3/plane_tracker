# Plane Tracker ✈️

A real-time flight tracking system running on an **ESP8266** (NodeMCU / ESP8266MOD) paired with an **ST7789 SPI TFT display**. 

Given a specific observer latitude and longitude, the device calculates a surrounding bounding box, queries the [OpenSky Network API](https://opensky-network.org/) over secure HTTPS, parses live ADS-B state vectors, and displays nearby aircraft details in real time.

---

## 🛠️ Hardware Requirements

| Component | Description |
| :--- | :--- |
| **Microcontroller** | ESP8266 (e.g. NodeMCU v2 / ESP-12E / ESP8266MOD) |
| **Display** | 240×280 ST7789 IPS TFT Display (SPI) |
| **Power Supply** | Micro-USB cable / 5V external power |
| **Wiring** | Breadboard and jumper wires |

---

## 🔌 Wiring & Pinout

The ST7789 display communicates via the ESP8266 hardware SPI bus:

| ST7789 Pin | ESP8266 Pin | GPIO | Function / Note |
| :--- | :--- | :--- | :--- |
| **VCC** | `3V3` | - | 3.3V Power |
| **GND** | `GND` | - | Ground |
| **SCL / SCK** | `D5` | GPIO14 | Hardware SPI Clock |
| **SDA / MOSI** | `D7` | GPIO13 | Hardware SPI Data |
| **CS** | `D8` | GPIO15 | Chip Select (`TFT_CS`) |
| **DC** | `D1` | GPIO5 | Data / Command (`TFT_DC`) |
| **RES / RST** | `D2` | GPIO4 | Reset (`TFT_RST`) |
| **BLK / BL** | `3V3` | - | Backlight (or GPIO if PWM dimming) |

---

## 📂 Project Architecture

```
plane_tracker/
├── lib/
│   ├── OpenSkyNetwork/      # OpenSky Network API client & JSON parsing
│   │   ├── openskynetwork.h
│   │   └── openskynetwork.cpp
│   └── WifiConnection/      # Wi-Fi connection handling
│       ├── wifi_connection.h
│       └── wifi_connection.cpp
├── src/
│   └── main.cpp             # Main setup, loop, and display rendering logic
├── include/                 # Shared header files
├── platformio.ini           # PlatformIO project configuration & dependencies
└── README.md
```

### Key Modules

* **`openskynetwork`**:
  * Calculates geographical bounding boxes (`lamin`, `lomin`, `lamax`, `lomax`) for a given radius around the target location.
  * Handles OAuth2 / Bearer token authentication with OpenSky Network.
  * Sends HTTPS GET requests to the OpenSky `/api/states/all` endpoint using `BearSSL::WiFiClientSecure`.
  * Parses state vectors with **ArduinoJson v7** into structured `Plane` objects (callsign, country of origin, altitude, velocity, and distance).

* **`wifi_connection`**:
  * Manages connection to the 2.4 GHz Wi-Fi network and reports status.

---

## 🚀 Getting Started

### Prerequisites

* [PlatformIO](https://platformio.org/) installed (as a VS Code / CLion extension or CLI).
* An [OpenSky Network](https://opensky-network.org/) account (recommended for higher API rate limits and OAuth2 credentials).

### Configuration

Before uploading to your ESP8266, configure your local settings in `src/main.cpp`:

1. **Wi-Fi Credentials:**
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```

2. **Observer Coordinates:**
   Set the coordinates of your monitoring location:
   ```cpp
   constexpr float latitude  = 49.4440386;
   constexpr float longitude = 11.0838801;
   ```

3. **OpenSky Token / Authentication:**
   * Enter your bearer token or use `openskynetwork::getBearerAuthToken` with your OpenSky client credentials.

---

## 📦 Building and Flashing

Build and flash using PlatformIO:

```bash
# Compile project
pio run

# Upload to connected ESP8266
pio run --target upload

# Open Serial Monitor (9600 baud)
pio run --target monitor
```

---

## 🧰 Dependencies

Managed automatically by PlatformIO via `platformio.ini`:

* [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library) (`^1.12.6`)
* [Adafruit ST7735 and ST7789 Library](https://github.com/adafruit/Adafruit-ST7735-Library) (`^1.11.0`)
* [ArduinoJson](https://arduinojson.org/) (`^7.2.2`)

---

## 📄 License

This project is open-source and available under the [MIT License](LICENSE).

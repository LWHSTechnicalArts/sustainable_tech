#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include "Adafruit_ThinkInk.h"

// ── Config ────────────────────────────────────────────────
const char* WIFI_SSID     = "StudentNet";
const char* WIFI_PASSWORD = "WIFI PASSWORD HERE";
const char* GROQ_API_KEY  = "get an API KEY from https://groq.com/ and paste here";
const char* GROQ_MODEL = "llama-3.1-8b-instant";

const float LATITUDE  = 37.7749;
const float LONGITUDE = -122.4194;

// ── eInk display ──────────────────────────────────────────
#define EPD_DC    10
#define EPD_CS    9
#define EPD_BUSY  -1
#define SRAM_CS   6
#define EPD_RESET -1
#define EPD_SPI   &SPI

ThinkInk_213_Mono_GDEY0213B74 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY, EPD_SPI);

// ── Groq root CA ──────────────────────────────────────────
const char* rootCA = R"(
-----BEGIN CERTIFICATE-----
MIIDejCCAmKgAwIBAgIQf+UwvzMTQ77dghYQST2KGzANBgkqhkiG9w0BAQsFADBX
MQswCQYDVQQGEwJCRTEZMBcGA1UEChMQR2xvYmFsU2lnbiBudi1zYTEQMA4GA1UE
CxMHUm9vdCBDQTEbMBkGA1UEAxMSR2xvYmFsU2lnbiBSb290IENBMB4XDTIzMTEx
NTAzNDMyMVoXDTI4MDEyODAwMDA0MlowRzELMAkGA1UEBhMCVVMxIjAgBgNVBAoT
GUdvb2dsZSBUcnVzdCBTZXJ2aWNlcyBMTEMxFDASBgNVBAMTC0dUUyBSb290IFI0
MHYwEAYHKoZIzj0CAQYFK4EEACIDYgAE83Rzp2iLYK5DuDXFgTB7S0md+8Fhzube
Rr1r1WEYNa5A3XP3iZEwWus87oV8okB2O6nGuEfYKueSkWpz6bFyOZ8pn6KY019e
WIZlD6GEZQbR3IvJx3PIjGov5cSr0R2Ko4H/MIH8MA4GA1UdDwEB/wQEAwIBhjAd
BgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDwYDVR0TAQH/BAUwAwEB/zAd
BgNVHQ4EFgQUgEzW63T/STaj1dj8tT7FavCUHYwwHwYDVR0jBBgwFoAUYHtmGkUN
l8qJUC99BM00qP/8/UswNgYIKwYBBQUHAQEEKjAoMCYGCCsGAQUFBzAChhpodHRw
Oi8vaS5wa2kuZ29vZy9nc3IxLmNydDAtBgNVHR8EJjAkMCKgIKAehhxodHRwOi8v
Yy5wa2kuZ29vZy9yL2dzcjEuY3JsMBMGA1UdIAQMMAowCAYGZ4EMAQIBMA0GCSqG
SIb3DQEBCwUAA4IBAQAYQrsPBtYDh5bjP2OBDwmkoWhIDDkic574y04tfzHpn+cJ
odI2D4SseesQ6bDrarZ7C30ddLibZatoKiws3UL9xnELz4ct92vID24FfVbiI1hY
+SW6FoVHkNeWIP0GCbaM4C6uVdF5dTUsMVs/ZbzNnIdCp5Gxmx5ejvEau8otR/Cs
kGN+hr/W5GvT1tMBjgWKZ1i4//emhA1JG1BbPzoLJQvyEotc03lXjTaCzv8mEbep
8RqZ7a2CPsgRbuvTPBwcOMBBmuFeU88+FSBX6+7iP0il8b4Z0QFqIwwMHfs/L6K1
vepuoxtGzi4CZ68zJpiq1UvSqTbFJjtbD4seiMHl
-----END CERTIFICATE-----
)";

// ── Weather struct ────────────────────────────────────────
struct Weather {
  float temperature_f;
  float temperature_c;
  float windspeed;
  float precipitation;
  String description;
  bool valid;
};

// ── Weather code → description ────────────────────────────
String weatherCodeToDescription(int code) {
  if (code == 0)  return "Clear sky";
  if (code <= 2)  return "Partly cloudy";
  if (code == 3)  return "Overcast";
  if (code <= 49) return "Foggy";
  if (code <= 59) return "Drizzling";
  if (code <= 69) return "Raining";
  if (code <= 79) return "Snowing";
  if (code <= 82) return "Rain showers";
  if (code <= 99) return "Thunderstorms";
  return "Unknown";
}

// ── Fetch weather ─────────────────────────────────────────
Weather fetchWeather() {
  Weather w; w.valid = false;
  HTTPClient http;
  String url = "http://api.open-meteo.com/v1/forecast"
               "?latitude=" + String(LATITUDE, 4) +
               "&longitude=" + String(LONGITUDE, 4) +
               "&current=temperature_2m,precipitation,weathercode,windspeed_10m"
               "&temperature_unit=celsius&wind_speed_unit=mph&forecast_days=1";
  http.begin(url);
  if (http.GET() == 200) {
    StaticJsonDocument<1024> doc;
    if (!deserializeJson(doc, http.getString())) {
      w.temperature_c = doc["current"]["temperature_2m"];
      w.temperature_f = (w.temperature_c * 9.0 / 5.0) + 32.0;
      w.windspeed     = doc["current"]["windspeed_10m"];
      w.precipitation = doc["current"]["precipitation"];
      w.description   = weatherCodeToDescription(doc["current"]["weathercode"].as<int>());
      w.valid         = true;
    }
  }
  http.end();
  return w;
}

// ── Ask Groq ──────────────────────────────────────────────
String getClothingAdvice(Weather& w) {
  WiFiClientSecure secureClient;
  secureClient.setCACert(rootCA);

  HTTPClient http;
  http.begin(secureClient, "https://api.groq.com/openai/v1/chat/completions");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + GROQ_API_KEY);
  http.setTimeout(15000);

  String prompt =
    "Weather in San Francisco: " + w.description + ", " +
    String(w.temperature_f, 1) + "F (" + String(w.temperature_c, 1) + "C), " +
    "wind " + String(w.windspeed, 1) + " mph, precip " +
    String(w.precipitation, 1) + " mm. " +
    "Give clothing advice in exactly 2 short sentences. Plain text only, no bullet points.";

  StaticJsonDocument<768> reqDoc;
  reqDoc["model"]      = GROQ_MODEL;
  reqDoc["max_tokens"] = 80;

  JsonArray messages = reqDoc.createNestedArray("messages");
  JsonObject sys     = messages.createNestedObject();
  sys["role"]        = "system";
  sys["content"]     = "Give brief practical clothing advice. 2 sentences max. Plain text only.";
  JsonObject usr     = messages.createNestedObject();
  usr["role"]        = "user";
  usr["content"]     = prompt;

  String body; serializeJson(reqDoc, body);
  String result = "";

  if (http.POST(body) == 200) {
    StaticJsonDocument<2048> res;
    if (!deserializeJson(res, http.getString())) {
      result = res["choices"][0]["message"]["content"].as<String>();
    }
  }
  http.end();
  return result;
}

// ── Word-wrap text onto display ───────────────────────────
void drawWrappedText(String text, int x, int y, int maxChars, int lineHeight) {
  String word = "";
  String line = "";
  text += " ";

  for (int i = 0; i < (int)text.length(); i++) {
    char c = text[i];
    if (c == ' ' || c == '\n') {
      if ((int)(line.length() + word.length()) <= maxChars) {
        line += word + " ";
      } else {
        display.setCursor(x, y);
        display.print(line);
        y += lineHeight;
        line = word + " ";
      }
      word = "";
      if (c == '\n') {         // force line break
        display.setCursor(x, y);
        display.print(line);
        y += lineHeight;
        line = "";
      }
    } else {
      word += c;
    }
    if (y > 118) break;        // stay on screen
  }
  if (line.length() > 0) {
    display.setCursor(x, y);
    display.print(line);
  }
}

// ── Render full screen ────────────────────────────────────
void renderDisplay(Weather& w, String advice) {
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);

  // Title bar
  display.fillRect(1, 0, 250, 22, EPD_BLACK);
  display.setTextColor(EPD_WHITE);
  display.setTextSize(1);
  display.setCursor(4, 12);
  display.print("SF Weather Advisor");

  // Conditions
  display.setTextColor(EPD_BLACK);
  display.setTextSize(1);
  display.setCursor(4, 22);
  display.print(w.description);
  display.setCursor(4, 32);
  display.print(String(w.temperature_f, 0) + "F  " +
                String(w.windspeed, 0) + "mph wind  " +
                String(w.precipitation, 1) + "mm");

  // Divider
  display.drawLine(0, 42, 250, 42, EPD_BLACK);

  // Advice text — 41 chars wide at size 1, 10px line height
  drawWrappedText(advice, 4, 45, 41, 10);

  display.display();
}

// ── Status message while loading ──────────────────────────
void showStatus(String msg) {
  display.clearBuffer();
  display.setTextColor(EPD_BLACK);
  display.setTextSize(1);
  display.setCursor(4, 55);
  display.print(msg);
  display.display();
}

// ── Wi-Fi ─────────────────────────────────────────────────
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConnected: " + WiFi.localIP().toString());
}

// ── Main run ──────────────────────────────────────────────
void run() {
  showStatus("Fetching weather...");
  Weather w = fetchWeather();

  if (!w.valid) {
    showStatus("Weather fetch failed.");
    return;
  }

  showStatus("Asking Groq...");
  String advice = getClothingAdvice(w);
  Serial.println("Advice: " + advice);

  renderDisplay(w, advice);
}

// ── Setup & Loop ──────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1000);
  display.begin(THINKINK_MONO);

  // Clean full refresh to clear any panel artifacts
  display.clearBuffer();
  display.display();
  delay(2000);

  Serial.println("Display ready");
  connectWiFi();
  run();
}

void loop() {
  delay(30UL * 60UL * 1000UL); // refresh every 30 minutes
  run();
}

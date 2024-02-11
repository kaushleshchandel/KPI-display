#include "Arduino.h"
#include "Arduino_GFX_Library.h"
#include "img_logo.h"
#include "WiFi.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
 

const char *ssid = "AAA";
const char *password = "608980608980";
const char *mqtt_server = "monitor.boondockecho.com";
const char *mqtt_user = "service";
const char *mqtt_password = "c5WTn3ivp5vs";
const int mqtt_port = 1883;
const char *mqtt_topic = "#"; // Change to your MQTT topic

WiFiClient espClient;
PubSubClient client(espClient);

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#error "The current version is not supported for the time being, please use a version below Arduino ESP32 3.0"
#endif

#define GFX_DEV_DEVICE LILYGO_T_DISPLAY_S3
#define GFX_EXTRA_PRE_INIT()              \
    {                                     \
        pinMode(15 /* PWD */, OUTPUT);    \
        digitalWrite(15 /* PWD */, HIGH); \
    }
#define GFX_BL 38
Arduino_DataBus *bus = new Arduino_ESP32PAR8Q(
    7 /* DC */, 6 /* CS */, 8 /* WR */, 9 /* RD */,
    39 /* D0 */, 40 /* D1 */, 41 /* D2 */, 42 /* D3 */, 45 /* D4 */, 46 /* D5 */, 47 /* D6 */, 48 /* D7 */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 5 /* RST */, 0 /* rotation */, true /* IPS */, 170 /* width */, 320 /* height */, 35 /* col offset 1 */, 0 /* row offset 1 */, 35 /* col offset 2 */, 0 /* row offset 2 */);

int32_t w, h, n, n1, cx, cy, cx1, cy1, cn, cn1;
uint8_t tsa, tsb, tsc, ds;

void reconnectWiFi()
{

    gfx->setCursor(50, 30);
    // gfx->clear;
    gfx->setTextSize(2);
    gfx->setTextColor(BLUE);
    // Clear the screen
    gfx->fillScreen(BLACK); // Replace BLACK with the background color of your choice
    gfx->print("Connecting WiFi...");
    
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    Serial.println("Connected to WiFi");
}

void reconnect()
{
    
    gfx->setCursor(50, 30);
    // gfx->clear;
    gfx->setTextSize(2);
    gfx->setTextColor(BLUE);
    // Clear the screen
    gfx->fillScreen(BLACK); // Replace BLACK with the background color of your choice
    gfx->print("Connecting MQTT...");
    

    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Client"))
        {
            Serial.println("connected");
            client.subscribe(mqtt_topic); // Subscribe to the topic
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

String raised;
String backers;
bool forceRefersh = false;

void callback(char *topic, byte *message, unsigned int length)
{
    String stopic = topic;
    String smessage = "";

    for (int i = 0; i < length; i++)
    {
        smessage = smessage + (char)message[i];
    }

    if (stopic.startsWith("crowdsupply/raised"))
    {
        Serial.println("Updating Raised amount");
        raised = smessage;
        forceRefersh = true;
    }

    if (stopic.startsWith("crowdsupply/backers"))
    {
        Serial.println("Updating Backers ");
        backers = smessage;
    }

    Serial.print("Message arrived on topic: " + stopic);
    Serial.print(topic);
    Serial.println(smessage);
}

void refreshDisplay()
{
    forceRefersh = false;
    gfx->setCursor(50, 30);
    // gfx->clear;
    gfx->setTextSize(6);
    gfx->setTextColor(WHITE);
    // Clear the screen
    gfx->fillScreen(BLACK); // Replace BLACK with the background color of your choice
    gfx->print(raised);
    gfx->setCursor(50, 120);
    // gfx->clear;
    gfx->setTextSize(3);
    gfx->setTextColor(YELLOW);
    gfx->print(backers);
}

void setup()
{

    Serial.begin(9600);

    GFX_EXTRA_PRE_INIT();

#ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
#endif

    gfx->begin();
    gfx->setRotation(1);
 
    reconnectWiFi();

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

static inline uint32_t micros_start() __attribute__((always_inline));
static inline uint32_t micros_start()
{
    uint8_t oms = millis();
    while ((uint8_t)millis() == oms)
        ;
    return micros();
}


unsigned long previousMillis = 0; // will store last time the display was updated
const long interval = 10000;      // interval at which to refresh display (milliseconds)

void loop(void)
{

    if (!WiFi.isConnected())
        reconnectWiFi();

    if (!client.connected())
    {
        reconnect();
    }

    client.loop();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        refreshDisplay();
    }

    if(forceRefersh )
     refreshDisplay();
        
    
}

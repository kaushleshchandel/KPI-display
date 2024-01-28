#include "Arduino.h"
#include "Arduino_GFX_Library.h"
#include "img_logo.h"
#include "WiFi.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

const char *ssid = "AAA";
const char *password = "608980608980";
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

String raised;
String backers;

void fetchPledgedAmount()
{

    WiFiClientSecure client;
    // Uncomment the next line to bypass SSL certificate verification (not recommended for production)
    client.setInsecure();

    HTTPClient http;
    String url = "https://www.crowdsupply.com/boondock-technologies/boondock-echo";

    http.begin(client, url);
    int httpCode = http.GET();

    if (httpCode == 200)
    {
        String payloadraised = http.getString();
        String payloadbackers = payloadraised;

        int startIndexRaised = payloadraised.indexOf("<p class=\"project-pledged\">");

        if (startIndexRaised != -1)
        {
            int endIndex = payloadraised.indexOf("</p>", startIndexRaised);
            String pledgedSection = payloadraised.substring(startIndexRaised, endIndex);
            int spanStart = pledgedSection.indexOf("<span>");
            int spanEnd = pledgedSection.indexOf("</span>", spanStart);
            String amountRaised = pledgedSection.substring(spanStart + 6, spanEnd);
            amountRaised.replace("<sup>", "");
            amountRaised.replace("</sup>", "");
            raised = amountRaised;

            //  Serial.println("Amount Raised: " + amountRaised);
        }
        else
        {
            raised = "-";
            //  Serial.println("Raised amount element not found.");
        }

        //<a href="/boondock-technologies/boondock-echo/backers">
        int startIndexBackers = payloadbackers.indexOf("<a href=\"/boondock-technologies/boondock-echo/backers\"");

        // Fetch the Backers
        if (startIndexBackers != -1)
        {
            int endIndex = payloadbackers.indexOf("\"fact-label\">", startIndexBackers);
            String backerSection = payloadbackers.substring(startIndexBackers, endIndex);
        
            int spanStart = backerSection.indexOf("fact-number\">");
            int spanEnd = backerSection.indexOf("</span>", spanStart);
            String amountbackers = backerSection.substring(spanStart + 6, spanEnd);
             amountbackers.replace("umber\">", "");
            //  amountbackers.replace("</sup>", "");
            backers = "Backers " + amountbackers ;

            
            Serial.println(backers);
        }
        else
        {
            backers = "-";
            Serial.println("Backers amount element not found.");
        }
    }
    else
    {
        backers = "-";
        Serial.print("Failed to retrieve webpage, status code: ");
        Serial.println(httpCode);
    }

    http.end();
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

    w = gfx->width();
    h = gfx->height();
    n = min(w, h);
    n1 = n - 1;
    cx = w / 2;
    cy = h / 2;
    cx1 = cx - 1;
    cy1 = cy - 1;
    cn = min(cx1, cy1);
    cn1 = cn - 1;
    tsa = ((w <= 176) || (h <= 160)) ? 1 : (((w <= 240) || (h <= 240)) ? 2 : 3); // text size A
    tsb = ((w <= 272) || (h <= 220)) ? 1 : 2;                                    // text size B
    tsc = ((w <= 220) || (h <= 220)) ? 1 : 2;                                    // text size C
    ds = (w <= 160) ? 9 : 12;

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    Serial.println("Connected to WiFi");
}

static inline uint32_t micros_start() __attribute__((always_inline));
static inline uint32_t micros_start()
{
    uint8_t oms = millis();
    while ((uint8_t)millis() == oms)
        ;
    return micros();
}

void loop(void)
{
    Serial.println(".");
    gfx->setCursor(50, 30);
    // gfx->clear;
    gfx->setTextSize(6);
    gfx->setTextColor(WHITE);
    // Clear the screen
    gfx->fillScreen(BLACK); // Replace BLACK with the background color of your choice
    fetchPledgedAmount();
    gfx->print(raised);

    gfx->setCursor(50, 120);
    // gfx->clear;
    gfx->setTextSize(3);
    gfx->setTextColor(YELLOW);
    gfx->print(backers);


    delay(300000);
}

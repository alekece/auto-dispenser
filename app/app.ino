#include "Secrets.h"
#include "UnipolarStepper.h"

#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <HX711.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

// Blynk related variables
BlynkTimer timer;
WidgetTerminal terminal(V7);
WidgetRTC rtc;
// hardwares related variables
UnipolarStepper stepper(32, D8, D6, D7, D5);
HX711 scale;
// logic variables
bool dispenseStarted = false;
bool appConnected = false;
int foodQuantity = 0;
float initialWeight = 0;
float currentWeight = 0;
int timeoutHandler = 0;

String getCurrentDate()
{
    return String(day()) + "/" + month() + "/" + year() + " " + hour() + ":" + minute() + ":" +
           second();
}

String getMacAddress()
{
    byte mac[6];

    WiFi.macAddress(mac);

    return String(mac[5], HEX) + ":" + String(mac[4], HEX) + ":" + String(mac[3], HEX) + ":" +
           String(mac[2], HEX) + ":" + String(mac[1], HEX) + ":" + String(mac[0], HEX);
}

void onDispenseStopped()
{
    dispenseStarted = false;

    stepper.setSpeed(0);
    Blynk.virtualWrite(V2, 0);

    // only shutdown the scale when the application isn't connected
    if (!appConnected) {
        scale.power_down();
    }
}

void onUpdate()
{
    if (scale.is_ready()) {
        currentWeight = scale.get_units();

        Blynk.virtualWrite(V1, static_cast<int>(currentWeight));

        if (dispenseStarted) {
            auto const foodDispensed = initialWeight - currentWeight;

            if (foodDispensed >= foodQuantity) {
                timer.deleteTimer(timeoutHandler);
                onDispenseStopped();

                terminal.print(getCurrentDate());
                terminal.print(" - ");
                terminal.print(static_cast<int>(initialWeight - currentWeight));
                terminal.println("g of food dispensed");
                terminal.flush();
            }
        }
    }
}

void onTimeout()
{
    onDispenseStopped();

    Blynk.notify("Alert! Food dispense fails!");

    terminal.print(getCurrentDate());
    terminal.print(" - Expected ");
    terminal.print(foodQuantity);
    terminal.print("g of food but ");
    terminal.print(static_cast<int>(initialWeight - currentWeight));
    terminal.println("g dispensed");
    terminal.flush();
}

BLYNK_CONNECTED()
{
    // fetch values from Blynk server
    Blynk.syncVirtual(V4, V6, V8);

    rtc.begin();

    terminal.clear();
    terminal.print(F("Blynk v" BLYNK_VERSION ": Device "));
    terminal.println(getMacAddress());
    terminal.println("----------");
    terminal.flush();
}

BLYNK_APP_CONNECTED()
{
    appConnected = true;

    scale.power_up();
}

BLYNK_APP_DISCONNECTED()
{
    appConnected = false;

    // avoid to stop the scale during food dispense
    if (!dispenseStarted) {
        scale.power_down();
    }
}

BLYNK_WRITE(V0)
{
    if (param.asInt()) {
        // startup the scale whatever the current state is
        scale.power_up();

        // initialize food dispense variables
        initialWeight = scale.get_units();
        dispenseStarted = true;
        // food dispense timeout after 10 seconds
        timeoutHandler = timer.setTimeout(10000, onTimeout);

        // then start stepper
        stepper.setSpeed(900);
        Blynk.virtualWrite(V2, 900);

        terminal.print(getCurrentDate());
        terminal.print(" - Dispense of ");
        terminal.print(foodQuantity);
        terminal.println("g of food begins");
        terminal.flush();
    }
}

BLYNK_WRITE(V2)
{
    stepper.setSpeed(param.asInt());
}

BLYNK_WRITE(V4)
{
    scale.set_scale(-param.asInt());
}

BLYNK_WRITE(V5)
{
    // reset the scale
    if (param.asInt()) {
        scale.set_scale();
        scale.tare();
        // store zero factor
        Blynk.virtualWrite(V6, scale.read_average());
    }
}

BLYNK_WRITE(V6)
{
    scale.set_offset(param.asInt());
}

BLYNK_WRITE(V8)
{
    foodQuantity = param.asInt();
}

void setup()
{
    scale.begin(D4, D3);
    scale.power_down();

    setSyncInterval(10 * 60);

    timer.setInterval(250, onUpdate);

    Blynk.begin(BLYNK_TOKEN, NETWORK_SSID, NETWORK_PASSWORD, SERVER_ADDRESS, SERVER_PORT);
}

void loop()
{
    Blynk.run();
    timer.run();
    // stepper instruction is out of update event because the rate isn't high enough
    // to get a linear speed with Blynk timer
    stepper.run();
}

#include <Arduino.h>

#define PROD_BRD_V1

#include <Objects.h>

#define BOARD_CONFIG 2

#define DEVICE_FIRMWARE_SKU = "GPIO001FULL";
#define DEVICE_FIRMWARE_KEY = "GPIO001";

#define VERSION "0.4.4"

#define DEBUG_MODE true

int sendCount = 0;
long lastSend = 0;
long lastPing = 0;
long lastGPSSample = 0;

unsigned long modemBaudRate = 115200;

bool gpsEnabed = false;

#define SENSOR_COUNT 6

AbstractSensor *sensors[SENSOR_COUNT];
void seaWolf_messagePublished_CallBack(String topic, unsigned char *payload, size_t length)
{
    String parts[10];
    int partIdx = 0;
    int start = 0;
    int topicLen = topic.length();
    for (int idx = 0; idx < topicLen; ++idx)
    {
        switch (topic[idx])
        {
        case '/':
            topic[idx] = 0x00;
            String segment = String(&topic[start]);
            parts[partIdx++] = segment;

            start = idx + 1;
            break;
        }
    }

    // Stick the final part where it belongs.
    String finalSegement = String(&topic[start]);
    parts[partIdx++] = finalSegement;

    if (partIdx == 5)
    {
        bool action = parts[4] == "on";

        if (parts[3] == "blower")
        {
            relayManager.setRelay(0, action);
        }
        else if (parts[3] == "bilge")
        {
            relayManager.setRelay(1, action);
        }
        else if (parts[3] == "tiltdown")
        {
            relayManager.setRelay(2, action);
        }
        else if (parts[3] == "tiltup")
        {
            relayManager.setRelay(3, action);
        }
    }
}

void sendStatusUpdate(String currentState, String nextAction, String title = "Commo Starting", int afterDelay = 0)
{
    display.drawStr(title.c_str(), currentState.c_str());
    delay(1000);
    display.drawStr(title.c_str(), nextAction.c_str());

    if (afterDelay > 0)
    {
        delay(afterDelay);
    }
}

/*
 * https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
 */

/*
 * https://medium.com/@supotsaeea/esp32-reboot-system-when-watchdog-timeout-4f3536bf17ef
 * 
 *  Implement watch dog reset timer.
 */

void setup()
{
    String firmwareSKU = "?";
    configPins.init(BOARD_CONFIG);

    console.setVerboseLogging(true);

    if (BOARD_CONFIG == 1)
    {
        consoleSerial.begin(115200, SERIAL_8N1);
        console.enableSerialOut(true);
    }
    else
    {
        consoleSerial.begin(115200, SERIAL_8N1, 5, 17);
        console.enableSerialOut(true);
    }

    console.println("WELCOME");
    console.println("SOFTWARE LOGISTICS FIRMWARE");

    if (BOARD_CONFIG == 1)
    {
        console.println("PROTOTYPE IOT MONITOR");
        firmwareSKU = "PIM001";
    }
    else if (BOARD_CONFIG == 2)
    {
        console.println("COMPACT IOT MONITOR");
        firmwareSKU = "CIM001";
    }
    else
    {
        console.println("UKNOWN BOARD TYPE");
    }

    console.println("VERSION: " + String(VERSION));

    display.enable(configPins.HasDisplay);

    display.prepare();
    display.setTextSize(1);
    display.clearBuffer();

    if (!twoWire.begin(21, 22, 400000))
    {
        display.drawStr("Could not start I2C.");
        console.println("Could not start I2C on pins 21 and 22");
        while (true)
            ;
    }
    else
    {
        console.println("Initialized I2C.");
    }

    if (!SPIFFS.begin(true))
    {
        display.drawStr("Could not initialize SPIFFS.");
        console.println("Could not initialize SPIFFS.");
        while (true)
            ;
    }
    else
    {
        console.println("Initialized SPIFFS.");
    }

    ioConfig.load();
    sysConfig.load();

    state.registerInt("modembaud", 115200);

    state.init(firmwareSKU, VERSION, "gp001", 001);

    sensors[0] = &adc;
    sensors[1] = &onOffDetector;
    sensors[2] = &pulseCounter;
    sensors[3] = &probes;
    sensors[4] = &relayManager;
    sensors[5] = &ledManager;

    for (int idx = 0; idx < SENSOR_COUNT; ++idx)
    {
        sensors[idx]->setup(&ioConfig);
    }

    modemBaudRate = state.getInt("modembaud");
    if (modemBaudRate == 0)
    {
        modemBaudRate = 115200;
    }

    //    gprsPort = HardwareSerial(configPins.SerialPort);7

    //if(configPins.UartNumber == 0) {
    //gprsPort.begin(modemBaudRate, SERIAL_8N1, configPins.SimRx, configPins.SimTx);
    gprsPort.begin(modemBaudRate, SERIAL_8N1);
    gprsPort.setRxBufferSize(16 * 1024);
    //}
    /*else if(configPins.UartNumber == 1) {
        gprsPort.begin(modemBaudRate, SERIAL_8N1, configPins.SimRx, configPins.SimTx);
        gprsPort.setRxBufferSize(16 * 1024);
    }
    else
    {
        ledManager.setErrFlashRate(1);
        while(true);
    }*/

    // Wait 5 seconds before

    ledManager.setOnlineFlashRate(1);
    ledManager.setErrFlashRate(1);

    delay(5000);

    ledManager.setErrFlashRate(0);
    ledManager.setOnlineFlashRate(8);

    client.setMessageReceivedCallback(seaWolf_messagePublished_CallBack);

    while (!sysConfig.Commissioned)
    {
        state.loop();
    }

    while (state.isValid())
    {
        if (!state.getIsConfigurationModeActive() && client.Connect(false, modemBaudRate))
        {
            ledManager.setOnlineFlashRate(2);

            mqtt.subscribe("nuviot/seawolf/" + sysConfig.DeviceId + "/#", QOS0);

            sendStatusUpdate("Subscribed to seawolf messages", "Ready");

            modem.startGPS();
            return;
        }

        state.loop();
    }
}

bool sendState()
{
    if (gps != NULL)
    {
        String gpsTopic = "nuviot/geo/" + sysConfig.DeviceId;

        sendStatusUpdate("Waiting", "Sending", "Message Transmission");

        mqtt.publish(gpsTopic, gps->getJSON(), QOS1);
    }

    String statusTopic = "nuviot/state/" + sysConfig.DeviceId;

    return mqtt.publish(statusTopic, payload->getJSON(), QOS1);
}

int idx = 0;

long lastDebugUpdate = 0;

void loop()
{
    if (!state.isValid())
    {
        state.loop();
    }
    else if (state.getIsConfigurationModeActive())
    {
        state.loop();
    }
    else if (state.getIsPaused())
    {
        display.clearBuffer();
        display.setTextSize(2);
        display.println("PAUSED");
        display.setTextSize(1);
        display.println("Device Id: " + sysConfig.DeviceId);
        display.println("IP: " + String(modem.getIPAddress()));
        display.sendBuffer();
        state.loop();
    }
    else
    {
        state.loop();
        mqtt.loop();

        for (int idx = 0; idx < SENSOR_COUNT; ++idx)
        {
            sensors[idx]->loop();
        }

        if (!sysConfig.GPSEnabled)
        {
            gps = NULL;
        }
        else if(((millis() - lastGPSSample)) > (sysConfig.GPSUpdateRate * 1000))
        {
            lastGPSSample = millis();
            gps = modem.readGPS();    
        }

        if (lastPing == 0 || ((millis() - lastPing) > sysConfig.PingRate * 1000))
        {
            lastPing = millis();

            if (!mqtt.ping())
            {
                ledManager.setOnlineFlashRate(8);
                ledManager.setErrFlashRate(8);

                console.println("connection=lost;");

                sendStatusUpdate("No MQTT Connection", "Restarting", "Message Loop", 1000);

                int retryCount = 10;
                while (!client.Connect(true, modemBaudRate)) {
                    retryCount--;
                    console.println("connection=failreconnect;");
                    if(retryCount == 0) {
                        hal.restart();
                    }
                }                    

                ledManager.setOnlineFlashRate(-1);
                ledManager.setErrFlashRate(0);

                mqtt.subscribe("seawolf/" + sysConfig.DeviceId + "/#", QOS0);

                sendCount = 0;
                lastSend = 0;
                lastPing = 0;
                return;
            }
            console.println("sent ping");
            ledManager.setOnlineFlashRate(-1);
        }

        if (lastSend == 0 || ((millis() - lastSend) > (sysConfig.SendUpdateRate * 1000) && state.isValid()))
        {
            lastSend = millis();
            sendCount++;

            sendState();
            sendStatusUpdate("Success", "Ready", "Message Transmission", 1000);

            display.setTextSize(1);
            display.clearBuffer();
            display.println("Device Id: " + sysConfig.DeviceId);
            display.println("IP: " + String(modem.getIPAddress()));
            display.println("Send message: " + String(sendCount));
            display.sendBuffer();
        }

        if ((millis() - lastDebugUpdate) > 1500)
        {
            console.setVerboseLogging(true);
            for (int idx = 0; idx < SENSOR_COUNT; ++idx)
            {
                sensors[idx]->debugPrint();
            }
            console.setVerboseLogging(false);
            lastDebugUpdate = millis();
        }
    }
}
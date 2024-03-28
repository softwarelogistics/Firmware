#define CAN_ENABLED
#define CAN_BRD_V1

#include <Arduino.h>
#include <NuvIoT.h>
#include "esp_err.h"
#include "EngineSensors.h"
#include "driver/gpio.h"
#include "driver/twai.h"

#define EXAMPLE_SKU "CAN_BUS_EXAMPLE"
#define FW_SKU "CAN_BRD_V1"
#define FIRMWARE_VERSION "0.6.3"

uint8_t count = 0;

CANEngineSensors sensor(&console);

TaskHandle_t commonLoopTask;
TaskHandle_t bleLoopTask;

void consoleHandler(String cmd)
{
  console.println("Command received: " + cmd);
  delay(100);
  twai_message_t message;
  message.identifier = 5;
  message.data_length_code = 8;
  for (int i = 0; i < 8; i++)
  {
    count++;
    message.data[i] = count;
    message.flags = TWAI_MSG_FLAG_NONE;
  }

  if(canBus.readStatus())
    ESP_ERROR_CHECK_WITHOUT_ABORT(twai_transmit(&message, pdMS_TO_TICKS(1000)));
}

void setup()
{
  initPins();

  configureConsole();
  configureFileSystem();

  canBus.setup();
  ioConfig.load();
  sysConfig.load();
  sysConfig.SendUpdateRateMS = 5000;

  ledManager.setup(&ioConfig);
  ledManager.setOnlineFlashRate(1);
  ledManager.setErrFlashRate(5);

  console.registerCallback(consoleHandler);

  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);

  String btName = "NuvIoT - " + (sysConfig.DeviceId == "" ? "CAN Bus BT Example" : sysConfig.DeviceId);
  BT.begin(btName.c_str(), FW_SKU);

  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "0.0.0", "cns001", 010);  

  state.registerBool("fuel", false);
  state.registerBool("cooling", false);
  state.registerBool("battery", false);
  state.printRemoteProperties();

  adc.setup(&ioConfig);
  pulseCounter.setup(&ioConfig);    

  if(sysConfig.CellEnabled) {
    console.println("Cellular Enabled");
    configureModem();
  }

  connect();

/*
  xTaskCreatePinnedToCore(
    communicationsTask,
    "Communications Task",
    10000,
    NULL,
    0, 
    &commonLoopTask,
    1);

  xTaskCreatePinnedToCore(
    bleTask,
    "Bluetooth Update Task",
    5000,
    NULL,
    0, 
    &bleLoopTask,
    1);
*/
}

int nextPrint = 0;
int idx = 0;

uint32_t __nextCanSend = 0;

uint16_t fuelRate = 1523;
uint16_t fuelPressure = 1423;
uint16_t manifoldPressure = 29.92;
bool bilgeOn = false;
bool highWaterDetection = false;
float coolantFlow = 80.4;


void sendBatteryLevels(int batteryInstance, float voltage)
{
  uint8_t buffer[8];
  buffer[0] = batteryInstance;
  int intVoltage = voltage * 100;
  buffer[1] = intVoltage & 0xFF;
  buffer[2] = (intVoltage >> 8) & 0xFF;
  buffer[3] = 0xFF;
  buffer[4] = 0xFF;
  buffer[5] = 0xFF;
  buffer[6] = 0xFF;
  buffer[7] = 0xFF;

  canBus.send(0x9F21440, buffer, 8);
}

void sendCustomEngineProperties()
{
  uint8_t buffer[8];
  buffer[0] = 0;
  buffer[1] = 1;
  buffer[2] = fuelRate & 0xFF;
  buffer[3] = (fuelRate >> 8) & 0xFF;
  buffer[4] = fuelPressure & 0xFF;
  buffer[5] = (fuelPressure >> 8) & 0xFF;
  buffer[6] = manifoldPressure & 0xFF;
  buffer[7] = (manifoldPressure >> 8) & 0xFF;
  canBus.send(0x9E20140, buffer, 8);

}

void sendCustom2Properties() {
  uint8_t buffer[8];
  for (uint8_t j = 1; j < 8; ++j)
    buffer[j] = 0xFF;

  buffer[0] = 0;
  buffer[1] = 1;
  buffer[2] = 0x00;
  if (bilgeOn)
    buffer[2] |= 0b00000010;

  if (highWaterDetection)
    buffer[2] |= 0b00000010;

  buffer[3] = (int)(coolantFlow * 100) & 0xFF;
  buffer[4] = ((int)(coolantFlow * 100) >> 8) & 0xFF;

  canBus.send(0x9E20240, buffer, 8);  
}

void loop()
{
  commonLoop();

  if (millis() > __nextCanSend)
  {
    __nextCanSend = millis() + 1000;

    if (canBus.readStatus())
    {
      if(state.getBool("fuel") || true) {
        sendCustomEngineProperties();        
        delay(150);
      }

      if(state.getBool("battery")) {    
        sendBatteryLevels(1, 12 + ((float)random(-10, 10) / 10.0));
        delay(150);
        sendBatteryLevels(2, 12 + ((float)random(-10, 10) / 10.0));
        delay(150);
        sendBatteryLevels(3, 12 + ((float)random(-10, 10) / 10.0));
        delay(150);
        sendBatteryLevels(4, 12 + ((float)random(-10, 10) / 10.0));
        delay(150);
        sendBatteryLevels(5, 12 + ((float)random(-10, 10) / 10.0));
        delay(150);
      }

      if(state.getBool("cooling"))
        sendCustom2Properties();
    }
  }
}
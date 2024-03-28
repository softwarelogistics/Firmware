#define PROD_BRD_V1_CAN
#define CAN_ENABLED

#include <Arduino.h>
#include <NuvIoT.h>
#include "driver/gpio.h"
#include "driver/twai.h"

#include <Wire.h>

#define TEMP_SNSR_SKU "RLY010"
#define FIRMWARE_VERSION "0.6.7"
#define HARDWARE_REVISION "5.0"

byte buffer[8];
bool running = true;


bool readStatus()
{
  twai_status_info_t status;
  ESP_ERROR_CHECK(twai_get_status_info(&status));

  if (status.state == TWAI_STATE_BUS_OFF)
  {
    console.println("TWAI_STATE_BUS_OFF: Tx err:" + String(status.tx_error_counter) + " Rx err:" + String(status.rx_error_counter));
    ESP_ERROR_CHECK_WITHOUT_ABORT(twai_initiate_recovery());
    return false;
  }

  if (status.state == TWAI_STATE_STOPPED)
  {
    console.println("TWAI_STATE_STOPPED");
    ESP_ERROR_CHECK_WITHOUT_ABORT(twai_start());
    return false;
  }

  if (status.state == TWAI_STATE_RECOVERING)
  {
    console.println("TWAI_STATE_RECOVERING");
    return false;
  }

  console.println("STATE_OK Tx err:" + String(status.tx_error_counter) + " Rx err:" + String(status.rx_error_counter));
  return true;
}

void sendBatteryLevels(int batteryInstance, float voltage)
{
  if(!readStatus())
    return;

  twai_message_t message;
  message.identifier = 0x9F21440;
  message.data_length_code = 8;
  message.extd = 1;

  message.data[0] = batteryInstance;
  int intVoltage = voltage * 100;
  message.data[1] = intVoltage & 0xFF;
  message.data[2] = (intVoltage >> 8) & 0xFF;
  message.data[3] = 0xFF;
  message.data[4] = 0xFF;
  message.data[5] = 0xFF;
  message.data[6] = 0xFF;
  message.data[7] = 0xFF;

  ESP_ERROR_CHECK_WITHOUT_ABORT(twai_transmit(&message, pdMS_TO_TICKS(1000)));
}


void setup(){
  delay(1000);
  configureConsole();

  state.init(TEMP_SNSR_SKU, FIRMWARE_VERSION, HARDWARE_REVISION, "pcl001", 010);
  configureFileSystem();  
  ioConfig.load();
  sysConfig.load();

  initPins();
  writeConfigPins();
  configureI2C();
  
  adc.setBankEnabled(1, true);
  adc.setBankEnabled(2, true);
  adc.setup(&ioConfig);
  pulseCounter.setup(&ioConfig);
  onOffDetector.setup(&ioConfig);

  ledManager.setup(&ioConfig);
  ledManager.setOnlineFlashRate(1);
  ledManager.setErrFlashRate(5);
  
  canBus.setup();

  welcome(TEMP_SNSR_SKU, FIRMWARE_VERSION);

  String btName = "NuvIoT - " + (sysConfig.DeviceId == "" ? "Relay Board" : sysConfig.DeviceId);

  BT.begin(btName.c_str(), "RELAY-005");

  wifiMgr.setup();  

  if(sysConfig.CellEnabled)
    configureModem();

  connect();
  console.println("setup=completed;");
}

void loop(){  
  commonLoop();
}
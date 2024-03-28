#define GENERIC

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "NuvIoT.h"

#include "WebRequestHandler.h"

#include "Settings_Module.h"
#include "WiFi_Module.h"

#include <WebServer.h>

#include "Common.h"
#include "PWM_Module.h"

#include "driver/ledc.h"

#define EXAMPLE_SKU "CAN_BUS_EXAMPLE"
#define FW_SKU "ENG-SIM-100"
#define FIRMWARE_VERSION "0.5.0"

//#define FORMAT_FILESYSTEM  // Uncomment to format the file system (only necessary once before first data upload)
// #define SHOW_PASSWORDS  // Uncomment to show passwords in debug serial output
// #define FORCE_AP  // Uncomment to force device into access-point mode at startup (don't try to connect to WiFi)

WiFi_Module* wifi;

WebServer *server;

PWM_Module *pwm = new PWM_Module(&console);

Settings_Module *settings;
WebRequestHandler *handler = new WebRequestHandler(pwm, settings, &console);


// EEPROM-backed settings
String ssid;
String password;
IPAddress local_ip;
IPAddress gateway;
IPAddress subnet;

// Specifies handles for the EEPROM-backed settings

bool wsIninitalized = false;

void setup() {
  initPins();

  configureConsole();
  configureFileSystem();
  welcome(EXAMPLE_SKU, FIRMWARE_VERSION);

  sysConfig.load();
  ioConfig.load();

  String btName = "NuvIoT - " + (sysConfig.DeviceId == "" ? "Engine Simulator" : sysConfig.DeviceId);
  BT.begin(btName.c_str(), FW_SKU);

  state.init(EXAMPLE_SKU, FIRMWARE_VERSION, "0.0.0", "engsim001", 010);  

  connect();

  pwm->Setup();
}

void loop() {
  pwm->Loop();
  commonLoop();
}
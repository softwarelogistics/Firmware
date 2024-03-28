#define NUM_SETTINGS 5
#define SSID_SETTING 0
#define PASSWORD_SETTING 1
#define LOCAL_IP_SETTING 2
#define GATEWAY_SETTING 3
#define SUBNET_SETTING 4

#define FILESYSTEM SPIFFS
#define DBG_OUTPUT_PORT Serial

#if FILESYSTEM == FFat
    #include <FFat.h>
#endif
#if FILESYSTEM == SPIFFS
    #include <SPIFFS.h>
#endif


extern String ssid;
extern String password;
extern IPAddress local_ip;
extern IPAddress gateway;
extern IPAddress subnet;

bool exists(String path);
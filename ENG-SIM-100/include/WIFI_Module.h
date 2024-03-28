#ifndef WiFi_Module_h
#define WiFi_Module_h

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

class WiFi_Module {
    public:
        bool TryConnect(String ssid, String password, IPAddress local_ip, IPAddress gateway, IPAddress subnet);
        IPAddress SetupAccessPoint();
        String GetIPAddress();
    private:
};

#endif
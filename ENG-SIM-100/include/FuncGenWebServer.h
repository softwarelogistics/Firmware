/*
#ifndef FUNC_GEN_WS
#define FUNC_GEN_WS

#include <Arduino.h>
#include <WebServer.h>
#include "PWM_Module.h"
#include "Console.h"
#include "Common.h"
#include "Settings_Module.h"
#include "driver/dac.h"
#include "driver/ledc.h"


class FuncGenWebServer {
    public: 
        FuncGenWebServer(PWM_Module *pwm, Settings_Module *settings, Console *console) {
            m_pwm = pwm;
            m_console = console;
            m_settings = settings;
        };

private:
    WebServer *m_server;
    Console *m_console;
    PWM_Module *m_pwm;
    Settings_Module *m_settings;


    class MethodHandler : RequestHandler {

        WebServer *m_server;
        Console *m_console;
        PWM_Module *m_pwm;
        Settings_Module *m_settings;
    
        // This method is invoked if the requested resource is not found
        void HandleNotFound() {
            if (!handleFileRead(m_server->uri())) {
                String message = "File Not Found\n\n";
                message += "URI: ";
                message += m_server->uri();
                message += "\nMethod: ";
                message += (m_server->method() == HTTP_GET) ? "GET" : "POST";
                message += "\nArguments: ";
                message += m_server->args();
                message += "\n";

                for (uint8_t i = 0; i < m_server->args(); i++) {
                    message += " " + m_server->argName(i) + ": " + m_server->arg(i) + "\n";
                }

                m_server->send(404, "text/plain", message);
            }
        };

        // This method is invoked to handle a reboot command
        void handleReboot() {
            m_server->send(200, "text/html", "Restarting...");
            ESP.restart();
        };


        // Get MIME content type from file extension
        String getContentType(String filename) {
            if (m_server->hasArg("download")) {
                return "application/octet-stream";
            } else if (filename.endsWith(".htm")) {
                return "text/html";
            } else if (filename.endsWith(".html")) {
                return "text/html";
            } else if (filename.endsWith(".css")) {
                return "text/css";
            } else if (filename.endsWith(".js")) {
                return "application/javascript";
            } else if (filename.endsWith(".png")) {
                return "image/png";
            } else if (filename.endsWith(".gif")) {
                return "image/gif";
            } else if (filename.endsWith(".jpg")) {
                return "image/jpeg";
            } else if (filename.endsWith(".ico")) {
                return "image/x-icon";
            } else if (filename.endsWith(".svg")) {
                return "image/svg+xml";
            } else if (filename.endsWith(".xml")) {
                return "text/xml";
            } else if (filename.endsWith(".pdf")) {
                return "application/x-pdf";
            } else if (filename.endsWith(".zip")) {
                return "application/x-zip";
            } else if (filename.endsWith(".gz")) {
                return "application/x-gzip";
            }
            return "text/plain";
        };

        // This method is invoked to handle signal generator setup
        void handleSetup() {
            // Shared parameters
            uint32_t phase = 0;
            uint32_t frequency = 1000;

            // PWM-specific parameters
            ledc_timer_t timer_num = LEDC_TIMER_0;
            ledc_channel_t pwm_channel = LEDC_CHANNEL_0;
            ledc_timer_bit_t resolution = LEDC_TIMER_10_BIT;
            bool highspeed = true;
            uint32_t duty = 512;
            uint32_t out_pin = 25;

            // DAC-specific parameters
            dac_channel_t dac_channel = DAC_CHANNEL_1;
            uint32_t clk_div = 0;
            uint32_t scale = 0;
            uint32_t invert = 2;

            String type = "square";
            for (uint8_t i = 0; i <m_server->args(); i++) {
                // String
                if (m_server->argName(i) == "type") { type =m_server->arg(i); continue; }

                // int
                if (m_server->argName(i) == "resolution") { resolution = (ledc_timer_bit_t)atoi(m_server->arg(i).c_str()); continue; }
                if (m_server->argName(i) == "timer_num") { timer_num = (ledc_timer_t)atoi(m_server->arg(i).c_str()); continue; }
                if (m_server->argName(i) == "pwm_channel") { pwm_channel = (ledc_channel_t)atoi(m_server->arg(i).c_str()); continue; }
                if (m_server->argName(i) == "dac_channel") { dac_channel = (dac_channel_t)atoi(m_server->arg(i).c_str()); continue; }

                // uint32_t
                if (m_server->argName(i) == "clk_div") { clk_div = strtoul(m_server->arg(i).c_str(), NULL, 10); continue; }
                if (m_server->argName(i) == "frequency") { frequency = strtoul(m_server->arg(i).c_str(), NULL, 10); continue; }
                if (m_server->argName(i) == "duty") { duty = strtoul(m_server->arg(i).c_str(), NULL, 10); continue; }
                if (m_server->argName(i) == "phase") { phase = strtoul(m_server->arg(i).c_str(), NULL, 10); continue; }
                if (m_server->argName(i) == "out_pin") { out_pin = strtoul(m_server->arg(i).c_str(), NULL, 10); continue; }
                if (m_server->argName(i) == "scale") { scale = strtoul(m_server->arg(i).c_str(), NULL, 10); continue; }
                if (m_server->argName(i) == "invert") { invert = strtoul(m_server->arg(i).c_str(), NULL, 10); continue; }

                // bool
                if (m_server->argName(i) == "highspeed") { highspeed = (m_server->arg(i) == "true"); continue; }

            m_server->send(400, "text/html", "Invalid parameter name: " +m_server->argName(i)); return;
            }

            if (type == "sine")
            {
                //dac->Setup(dac_channel, clk_div, frequency, scale, phase, invert);
            m_server->send(200, "text/html", "Sine wave setup successful.");
                return;
            }
            if (type == "square")
            {
                char buff[1024];
            m_pwm->Setup(timer_num, pwm_channel, highspeed, resolution, frequency, duty, phase, out_pin);
            m_server->send(200, "text/html", "Square wave setup successful.");
                return;
            }

            m_server->send(400, "text/html", "Invalid type.");
        };

            // This method is invoked to stop a signal generator
            void handleStop() {
                String type = "square";
                ledc_channel_t pwm_channel = LEDC_CHANNEL_0;
                dac_channel_t dac_channel = DAC_CHANNEL_1;
                bool highspeed = true;

                for (uint8_t i = 0; i <m_server->args(); i++) {
                    if (m_server->argName(i) == "type") { type =m_server->arg(i); continue; }
                    if (m_server->argName(i) == "pwm_channel") { pwm_channel = (ledc_channel_t)atoi(m_server->arg(i).c_str()); continue; }
                    if (m_server->argName(i) == "dac_channel") { dac_channel = (dac_channel_t)atoi(m_server->arg(i).c_str()); continue; }
                    if (m_server->argName(i) == "highspeed") { highspeed = (m_server->arg(i) == "true"); continue; }
                m_server->send(400, "text/html", "Invalid parameter name: " +m_server->argName(i)); return;
                }

                if (type == "sine")
                {
                    dac->Stop(dac_channel);
                    m_server->send(200, "text/html", "Sine wave stopped successfully.");
                }
                else if (type == "square")
                {
                    m_pwm->Stop(pwm_channel, highspeed);
                    m_server->send(200, "text/html", "Square wave stopped successfully.");
                }
                else   
                    m_server->send(400, "text/html", "Invalid type.");
            };

            // This method is invoked to get or set the configuration
            void handleConfig() {
                switch (m_server->method())
                {
                case HTTP_GET:
                    Serial.println("Getting configuration...");

                m_server->send(200, "text/html", "{\"ssid\":\"" + ssid + "\"," + "\"local_ip\":\"" + local_ip.toString() + "\"," + "\"gateway\":\"" + gateway.toString() + "\"," + "\"subnet\":\"" + subnet.toString() + "\"}");
                    break;
                case HTTP_POST:
                    Serial.println("Setting configuration...");

                    for (uint8_t i = 0; i <m_server->args(); i++) {
                        if (m_server->argName(i) == "ssid"){
                            ssid =m_server->arg(i);
                            Serial.println("Setting ssid to '" + ssid + "'");
                            m_settings->StoreString(SSID_SETTING, ssid);
                        } else if (m_server->argName(i) == "password"){
                            password =m_server->arg(i);
                            Serial.print("Setting password to '");
                            #ifdef SHOW_PASSWORDS
                                Serial.print(password);
                            #else
                                Serial.print("****");
                            #endif
                            Serial.println("'");
                            m_settings->StoreString(PASSWORD_SETTING, password);
                        } else if (m_server->argName(i) == "local_ip"){
                            local_ip.fromString(m_server->arg(i));
                            Serial.println("Setting local_ip to '" + local_ip.toString() + "'");
                            m_settings->StoreIp(LOCAL_IP_SETTING, local_ip);
                        } else if (m_server->argName(i) == "gateway"){
                            gateway.fromString(m_server->arg(i));
                            Serial.println("Setting gateway to '" + gateway.toString() + "'");
                            m_settings->StoreIp(GATEWAY_SETTING, gateway);
                        } else if (m_server->argName(i) == "subnet"){
                            subnet.fromString(m_server->arg(i));
                            Serial.println("Setting subnet to '" + subnet.toString() + "'");
                            m_settings->StoreIp(SUBNET_SETTING, subnet);
                        } else {
                        m_server->send(400, "text/html", "Invalid parameter: " +m_server->argName(i));
                        }
                    }
                    m_settings->Commit();

                m_server->send(200, "text/html", "Configuration updated.");
                    break;
                default:
                m_server->send(405, "text/html", "Invalid method.");
                    break;
                }
            };


        // This method is invoked to handle GET requests on static files
        bool handleFileRead(String path) {
            if (path.endsWith("/")) {
                path += "index.html";
            }
            String contentType = getContentType(path);
            if (exists(path)) {
                File file = FILESYSTEM.open(path, "r");
                m_server->streamFile(file, contentType);
                file.close();
                return true;
            }
            return false;
        };
    }

    public:
    // Initialize the web server and set up the API endpoints
    void SetupWebserver() {
        m_server = new WebServer(80);
        m_server->on("/config",() {this.handleConfig()});
        m_server->on("/setup", handleSetup);
        m_server->on("/stop", handleStop);
        m_server->on("/reboot", handleReboot);
        m_server->onNotFound(HandleNotFound);
        m_server->begin();
    };
};

#endif*/
#ifndef WEB_REQUESTHANDLER
#define WEB_REQUESTHANDLER

#include <WebServer.h>
#include "PWM_Module.h"
#include "Console.h"
#include "Common.h"
#include "Settings_Module.h"
#include "driver/dac.h"
#include "driver/ledc.h"

class WebRequestHandler : public RequestHandler {
        WebServer *m_server;
        Console *m_console;
        PWM_Module *m_pwm;
        Settings_Module *m_settings;

    public: 
        WebRequestHandler(PWM_Module *pwm, Settings_Module *settings, Console *console) {
            m_pwm = pwm;
            m_console = console;
            m_settings = settings;
        };

     bool canHandle(HTTPMethod method, String uri) {
        return true;
     };

     bool exists(String path){
        bool yes = false;
        if(!SPIFFS.exists(path)){
            m_console->println("file " + path + " not found");
            return false;
        }
            
        File file = SPIFFS.open(path, "r");
        if (!file.isDirectory()) {
            m_console->println("file exists " + path + ";");
            yes = true;
        }
        file.close();
        return yes;
    }

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

    bool handleFileRead(String path, WebServer& server) {
        if (path.endsWith("/")) {
            path += "index.html";
        }
        String contentType = getContentType(path);
        if (exists(path)) {
            return false;
            File file = SPIFFS.open(path, "r");
            server.streamFile(file, contentType);
            file.close();
            return true;
        }
        return false;
    };

    bool handleAPICommand(String path, WebServer &server) {
        return false;
    };

    bool canUpload(String uri) { 
        return true;
    };

    bool handle(WebServer& server, HTTPMethod requestMethod, String path) {
        if (path.endsWith("/")) {
            path += "index.html";
        }

        if(!exists(path)) {
            server.send(404, "text/html", "Not Found");
            return true;
        }

        if(handleFileRead(path, server)) 
            return true;

        //if(handleAPICommand(requestUri, server))
            //return true;

        m_console->println(path);
        server.send(404, "text/html", "Not Found");
        return true;
    };
    
    void upload(WebServer& server, String requestUri, HTTPUpload& upload) { 

    };
};

#endif

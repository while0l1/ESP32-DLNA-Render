#include "renderAction.h"
#include <ESP32SSDP.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFi.h>

// const char ssid[] = "LZH";
// const char pass[] = "2020&Apr:27";

const char ssid[] = "CMCC-4Td6";
const char password[] = "38300609956";

WebServer HTTP(80);

void handleNotFound()
{
    String message = "NOT FOUND\nURI: ";
    message += HTTP.uri();
    message += "\nMethod: ";
    message += (HTTP.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += HTTP.args();
    message += "\n";
    for (uint8_t i = 0; i < HTTP.args(); i++) {
        message += "" + HTTP.argName(i) + ": " + HTTP.arg(i) + "\n";
    }
    message += "\nHeaders: ";
    message += HTTP.headers();
    message += "\n";
    for (uint8_t i = 0; i < HTTP.headers(); i++) {
        message += "" + HTTP.headerName(i) + ": " + HTTP.header(i) + "\n";
    }

    HTTP.send(404, "text/plain", message);
    Serial.print(message);
}

void handleRenderingDesc()
{
    File file = SPIFFS.open("/RenderDesc.xml");
    HTTP.streamFile(file, "text/xml");
    file.close();
}

void handleAVTDesc()
{
    File file = SPIFFS.open("/AVDesc.xml");
    HTTP.streamFile(file, "text/xml");
    file.close();
}

void handleConnectDesc()
{
    File file = SPIFFS.open("/ConnectDesc.xml");
    HTTP.streamFile(file, "text/xml");
    file.close();
}

void handleRenderingAction()
{
    String action = "";
    String soapSend = "";
    for (uint8_t i = 0; i < HTTP.headers(); i++) {
        if (HTTP.headerName(i).equals("SOAPAction")) {
            action = HTTP.header(i).substring(49);
            break;
        }
    }
    int actionCode = parseActionHeader(action);
    switch (actionCode) {
    case CODE_GetVolume:
        soapSend = playerGetVolume();
        break;
    case CODE_SetVolume:
        soapSend = playerSetVolume(HTTP.arg(0));
        break;
    default:
        break;
    }
    Serial.printf("Ren: %s\n", action.c_str());
    Serial.println(soapSend);
    HTTP.send(200, "text/xml", soapSend);
}

void handleAVTAction()
{
    String action = "";
    String soapSend = "";
    for (uint8_t i = 0; i < HTTP.headers(); i++) {
        if (HTTP.headerName(i).equals("SOAPAction")) {
            // lem("\"urn:schemas-upnp-org:service:AVTransport:1#") = 44
            action = HTTP.header(i).substring(44);
            break;
        }
    }
    int actionCode = parseActionHeader(action);
    switch (actionCode) {
    case CODE_GetPositionInfo:
        soapSend = playerGetPosition();
        break;
    case CODE_Play:
        soapSend = playerPlay();
        break;
    case CODE_Pause:
        soapSend = playerPause();
        break;
    case CODE_SetAVTransportURI:
        soapSend = playerSetAVTransportURI(HTTP.arg(0));
        break;
    case CODE_Seek:
        playerSeek(HTTP.arg(0));
        break;
    default:
        break;
    }
    Serial.printf("Avt: %s\n", action.c_str());
    Serial.println(soapSend);
    // HTTP.sendHeader("Connection", "keep-alive");
    HTTP.send(200, "text/xml", soapSend);
}

void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("Starting WiFi...");

    SPIFFS.begin(true);

    playerInit();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {

        Serial.printf("Starting HTTP...\n");
        HTTP.on("/index.html", HTTP_GET, []() {
            HTTP.send(200, "text/plain", "Hello World!");
        });
        HTTP.on("/description.xml", HTTP_GET, []() {
            SSDP.schema(HTTP.client());
        });
        HTTP.on("/RenderingControl/desc.xml", handleRenderingDesc);
        HTTP.on("/AVTransport/desc.xml", handleAVTDesc);
        HTTP.on("/ConnectionManager/desc.xml", handleConnectDesc);

        HTTP.on("/RenderingControl/action", handleRenderingAction);
        HTTP.on("/AVTransport/action", handleAVTAction);
        // HTTP.on("/AVTransport/event", HTTP_ANY, []() {
        //     HTTP.send(200, "text/plain", "");
        // });
        // HTTP.on("/ConnectionManager/event", HTTP_ANY, []() {
        //     HTTP.send(200, "text/plain", "");
        // });
        HTTP.on("/RenderingControl/event", HTTP_ANY, []() {
            HTTP.send(200, "text/plain", "");
        });
        HTTP.onNotFound(handleNotFound);
        const char* headerkeys[] = { "SOAPAction" };
        size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
        HTTP.collectHeaders(headerkeys, headerkeyssize);
        HTTP.begin();

        //set schema xml url, nees to match http handler
        //"ssdp/schema.xml" if not set
        SSDP.setSchemaURL("description.xml");
        //set port
        //80 if not set
        SSDP.setHTTPPort(80);
        //set device name
        //Null string if not set
        SSDP.setName("ESP32 DLNA");
        //set Serial Number
        //Null string if not set
        // SSDP.setSerialNumber("001788102201");
        //set device url
        //Null string if not set
        // SSDP.setURL("index.html");
        //set model name
        //Null string if not set
        // SSDP.setModelName("Philips hue bridge 2012");
        //set model description
        //Null string if not set
        // SSDP.setModelDescription("This device can be controled by WiFi");
        //set model number
        //Null string if not set
        // SSDP.setModelNumber("929000226503");
        //set model url
        //Null string if not set
        // SSDP.setModelURL("http://www.meethue.com");
        //set model manufacturer name
        //Null string if not set
        // SSDP.setManufacturer("Royal Philips Electronics");
        //set model manufacturer url
        //Null string if not set
        // SSDP.setManufacturerURL("http://www.philips.com");
        //set device type
        //"urn:schemas-upnp-org:device:Basic:1" if not set
        SSDP.setDeviceType("urn:schemas-upnp-org:device:MediaRenderer:1"); //to appear as root device
        //set server name
        //"Arduino/1.0" if not set
        SSDP.setServerName("SSDPServer/1.0");
        //set UUID, you can use https://www.uuidgenerator.net/
        //use 38323636-4558-4dda-9188-cda0e6 + 4 last bytes of mac address if not set
        //use SSDP.setUUID("daa26fa3-d2d4-4072-bc7a-a1b88ab4234a", false); for full UUID
        SSDP.setUUID("74e8ae42-fa8d-11eb-9a03-0242ac130003");
        //Set icons list, NB: optional, this is ignored under windows
        SSDP.setIcons("<icon>"
                      "<mimetype>image/png</mimetype>"
                      "<height>48</height>"
                      "<width>48</width>"
                      "<depth>24</depth>"
                      "<url>icon48.png</url>"
                      "</icon>");
        //Set service list, NB: optional for simple device
        SSDP.setServices(
            "<service>"
            "<serviceType>urn:schemas-upnp-org:service:AVTransport:1</serviceType>"
            "<serviceId>urn:upnp-org:serviceId:AVTransport</serviceId>"
            "<controlURL>/AVTransport/action</controlURL>"
            "<eventSubURL>/AVTransport/event</eventSubURL>"
            "<SCPDURL>/AVTransport/desc.xml</SCPDURL>"
            "</service>"
            "<service>"
            "<serviceType>urn:schemas-upnp-org:service:RenderingControl:1</serviceType>"
            "<serviceId>urn:upnp-org:serviceId:RenderingControl</serviceId>"
            "<controlURL>/RenderingControl/action</controlURL>"
            "<eventSubURL>/RenderingControl/event</eventSubURL>"
            "<SCPDURL>/RenderingControl/desc.xml</SCPDURL>"
            "</service>"
            // "<service>"
            // "<serviceType>urn:schemas-upnp-org:service:ConnectionManager:1</serviceType>"
            // "<serviceId>urn:upnp-org:serviceId:ConnectionManager</serviceId>"
            // "<controlURL>/ConnectionManager/action</controlURL>"
            // "<eventSubURL>/ConnectionManager/event</eventSubURL>"
            // "<SCPDURL>/ConnectionManager/desc.xml</SCPDURL>"
            // "</service>"
        );

        Serial.printf("Starting SSDP...\n");
        SSDP.begin();

        Serial.printf("Ready!\n");
    } else {
        Serial.printf("WiFi Failed\n");
        while (1) {
            delay(100);
        }
    }
}

void loop()
{
    playerLoop();
    HTTP.handleClient();
    // delay(1);
}

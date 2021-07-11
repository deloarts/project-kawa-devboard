/*
    Project Kawa Client

    Date        2021.07.11
    Author      Philip Delorenzo
    License     None
    Repo        https://github.com/deloarts/project-kawa-devboard

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  DESCRIPTION
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    See README.md

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  IDE / SOFTWARE
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	SOFTWARE                VERSION         LINKS
	VS Code                 current         https://code.visualstudio.com/download
	Arduino IDE             1.8.15          https://www.arduino.cc/download_handler.php?f=/arduino-1.8.15-windows.exe
    SPIFFS FS for Arduino   0.5.0           https://github.com/esp8266/arduino-esp8266fs-plugin/releases
	Arduino for VS Code     0.3.0           https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino

	LIBRARY                 VERSION         INSTALLATION VIA            LINKS
	esp8266                 3.0.0           Arduino Board Manager       http://arduino.esp8266.com/stable/package_esp8266com_index.json
    ArduinoJson             6.18.0          Arduino Library Manager     https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
	OneWire                 2.3.3           Arduino Library Manager     https://www.pjrc.com/teensy/td_libs_OneWire.html
    DallasTemperature       3.9.0           Arduino Library Manager     https://github.com/milesburton/Arduino-Temperature-Control-Library
    Unified Sensor          1.1.4           Arduino Library Manager     https://github.com/adafruit/Adafruit_Sensor
    Adafruit_BME280         2.1.4 (mod)     Assets Folder               h̶t̶t̶p̶s̶:̶/̶/̶g̶i̶t̶h̶u̶b̶.̶c̶o̶m̶/̶a̶d̶a̶f̶r̶u̶i̶t̶/̶A̶d̶a̶f̶r̶u̶i̶t̶_̶B̶M̶E̶2̶8̶0̶_̶L̶i̶b̶r̶a̶r̶y̶

    Important note:
    Since the Adafruit_BMP280 library doesn't support sleep-mode it has been modified and
    stored in the assets folder. Keep that in mind when adding it to the c_cpp_properties.json.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  HARDWARE
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    See the schematics PDF.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  BOARD Config
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	Board:              Generic ESP8266 Module (esp8266)
	CPU Frequency:      160 MHz
    MMU:                32KB cache + 32KB IRAM
    Flash Frequency:    40 MHz
    Flash Mode:         DOUT
	Flash Size:         4M (FS:3MB, OTA:~512KB)
	Upload Speed:       115200
	Core Debug Level:   None

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  IMPORTANT NOTES
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -   
    Don't forget to upload the sketch data folder to the SPIFFS.
    Currently 'FS' is in use, but it's deprecated and will be replaced by
    'LittleFS'. Should be changed sometime.
    
    To upload the code you need to:
     - Press and hold the UPLOAD button
     - Press and release the RESET button
     - Release the UPLOAD button as soon as the process starts
     - Press the RESET button after the upload is done

    After uploading (serial) you need to hard reset the chip, otherwise the 
    ESP.reset function will fail.

    Don't forget to setup the include-paths in the c_cpp_properties.json
    
	Don't forget to setup the build path and the working file in the arduino.json

*/

#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1

#include <Arduino.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266httpUpdate.h>
#include <FS.h> // SPIFFS is depricated, see https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html#spiffs-deprecation-warning
// #include <LittleFS.h>

extern "C"
{
    #include "user_interface.h"    
    
    #include "headers/definitions.h"
    #include "headers/globals.h"
    #include "headers/util.h"
    #include "headers/sensor.h"
    #include "headers/power.h"
    #include "headers/filesystem.h"
    #include "headers/captive_portal.h"
    #include "headers/update.h"
}

ADC_MODE(ADC_VCC);

#pragma region DAILY BUSINESS

bool connect_wifi()
{
    serial_info_ln("Connecting to wifi... ");
    if (cfg.ssid[0] == 0)
    {
        serial_error_ln("Wifi credentials not set.");
        return false;
    }
    else
    {
        char hostname[100];
        get_hostname().toCharArray(hostname, sizeof(hostname));

        connection_time = millis();
        wifi_station_set_hostname(hostname);
        WiFi.begin(cfg.ssid, cfg.password);
        int result = WiFi.waitForConnectResult();
        connection_time = millis() - connection_time;

        serial_info_ln("Connected to Wifi:");
        serial_enum_ln("SSID:     " + String(cfg.ssid));
        serial_enum_ln("Password: " + String(cfg.password[0]) + "***");
        serial_enum_ln("Server:   " + String(cfg.address));
        serial_enum_ln("Port:     " + String(cfg.port));
        return true;
    }
}

void post_data()
{
    int8_t dbm = WiFi.RSSI();
    long response = 0;
    byte attempt = 0;
    String url = "http://" + String(cfg.address) + ":" + String(cfg.port) + api_post_data;

    char json_output[2048];
    DynamicJsonDocument doc(2048);
    JsonObject j_device  = doc.createNestedObject("device");
    JsonObject j_network  = doc.createNestedObject("network");
    JsonObject j_data  = doc.createNestedObject("data");

    JsonObject j_sensor  = j_device.createNestedObject("sensor");
    JsonObject j_battery  = j_device.createNestedObject("battery");
    JsonObject j_interval  = j_device.createNestedObject("interval");
    JsonObject j_connection  = j_device.createNestedObject("connection_time");

    JsonObject j_temperature = j_data.createNestedObject("temperature");
    JsonObject j_pressure = j_data.createNestedObject("pressure");
    JsonObject j_altitude = j_data.createNestedObject("altitude");
    JsonObject j_humidity = j_data.createNestedObject("humidity");

    j_device["id"] = get_device_id();
    j_device["boardname"] = board_name;
    j_device["firmware_version"] = firmware_version;
    j_device["filesystem_version"] = filesystem_version;
    j_device["mode"] = software_state;

    j_sensor["status"] = sensor_ok;
    j_sensor["name"] = cfg.sensor;

    j_battery["ok"] = voltage.ok;
    j_battery["value"] = voltage.value;
    j_battery["unit"] = voltage.unit;

    j_interval["value"] = cfg.interval * 60;
    j_interval["unit"] = unit_interval;

    j_connection["value"] = connection_time;
    j_connection["unit"] = unit_connection;

    j_network["ssid"] = cfg.ssid;
    j_network["dbm"] = dbm;
    j_network["quality"] = get_quality_from_dbm(dbm);
    j_network["hostname"] = get_hostname();
    j_network["ip"] = get_ip_address();
    j_network["mac"] = get_mac_address();

    j_temperature["available"] = temperature.available;
    j_temperature["value"] = temperature.value;
    j_temperature["unit"] = temperature.unit;
    
    j_pressure["available"] = pressure.available;
    j_pressure["value"] = pressure.value;
    j_pressure["unit"] = pressure.unit;
    
    j_altitude["available"] = altitude.available;
    j_altitude["value"] = altitude.value;
    j_altitude["unit"] = altitude.unit;
    
    j_humidity["available"] = humidity.available;
    j_humidity["value"] = humidity.value;
    j_humidity["unit"] = humidity.unit;

    serial_info("Serializing json: ");
    serializeJson(doc, json_output);
    serial_print_ln("OK");
    serial_enum_ln(json_output);
    
    http.begin(client, url);
	http.addHeader("Content-Type", "application/json");
	http.addHeader("Access-Control-Allow-Methods", "POST");

	serial_info_ln("Sending POST request to " + String(url) + " ...");
    while (response != 200)
    {
        response = http.POST(json_output);
        if (response == 200)
        {
            serial_info_ln("OK at attempt " + String(attempt));
            break;
        }
        else
        {
            if (attempt > max_retries_post)
            {
                serial_error_ln("Aborted because of too many attempts. Server did not answer.");
                return;
            }
            else
            {
                serial_enum_ln("Failed attempt " + String(attempt) + ". New attempt ...");
                attempt++;
                /*
                    The following delay causes about 65mAh of 
                    power loss. A better solution is required!
                    Maybe a oneshot without re-attempts?
                */
                delay(1000);
            }
        }
    }

    String server_answer = http.getString();
	http.end();

    serial_info("Server answer: ");
    serial_print_ln(server_answer);

    DynamicJsonDocument json_doc_root(128);
    deserializeJson(json_doc_root, server_answer);
    JsonObject j_root = json_doc_root.as<JsonObject>();

    /*
        Firmware update can't be started within the post-function,
        this causes a RAM overflow!
        Current solution: Global variable as trigger.
    */
    firmware.update_needed = j_root["update_firmware"];
    firmware.new_version = j_root["new_firmware"].as<String>();
    filesystem.update_needed = j_root["update_spiffs"];
    filesystem.new_version = j_root["new_spiffs"].as<String>();
}

#pragma endregion DAILY BUSINESS

#pragma region MAIN

void setup()
{       
    serial_init(115200);
    serial_print_ln("\n\nProject Kawa\n");
    serial_info_ln("Firmware " + String(firmware_version));
    serial_info_ln("SPIFFS   " + String(filesystem_version));

    serial_info_ln("Setting up IO...");
    pinMode(out_led, OUTPUT);
    pinMode(btn_1, INPUT_PULLUP);
    pinMode(btn_2, INPUT_PULLUP);
    /*
        Do not set btn_3 as INPUT or INPUT_PULLUP when onewire
        is in use. It's the same input and causes the onewire bus to fail.
    */
    // pinMode(btn_3, INPUT_PULLUP);
    digitalWrite(out_led, OFF);

    serial_info_ln("Setting up file system...");
    SPIFFS.begin();
    delay(100);

    serial_info_ln("Reading controller data...");
    get_voltage();

    serial_info_ln("Reading config file...");
    read_config();
    
    if (!digitalRead(btn_1))
    {
        serial_warning_ln("Button 1 was pressed during boot.");
        launch_ap();
    }
    else if (!digitalRead(btn_2))
    {
        serial_warning_ln("Button 2 was pressed during boot.");
        clear_config();
        digitalWrite(out_led, ON);
        delay(3000);
        digitalWrite(out_led, OFF);
        ESP.restart();
    }
    else if (connect_wifi())
    {
        sensor_ok = init_sensor();
        if (sensor_ok)
            get_sensor_data();
        
        post_data();
        unsigned long took = millis() - startup_time;
        serial_info_ln("Time since boot: " + String(took) + "ms.");

        if (firmware.update_needed)
            update_firmware(firmware.new_version);
        if (filesystem.update_needed)
            update_filesystem(filesystem.new_version);
    }
    else
    {
        /*
            Indicate that the WiFi isn't setup properly.
        */
       blink(25);
    }

    if (voltage.value < battery_too_low)
        enter_deep_sleep_max();
    else
        enter_deep_sleep(cfg.interval);
}

void loop() 
{
    /*
        Nothing to do here.
    */
    serial_warning_ln("This should not be possible!");
    delay(1000);
}

#pragma endregion MAIN

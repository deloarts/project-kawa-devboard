/*
    Project Kawa Client | FILESYSTEM

    Date        2021.07.11
    Author      Philip Delorenzo
    License     None
    Repo        https://github.com/deloarts/project-kawa-devboard

*/

void read_config()
{
    File config_file = SPIFFS.open(spiffs_config_file, "r");
    if (!config_file)
    {
        serial_warning_ln("File does not exist.");
    }
    else
    {
        size_t size = config_file.size();
        if (size == 0)
        {
            serial_warning_ln("File is empty.");
        }
        else
        {
            DynamicJsonDocument json_doc_root(2048);
            deserializeJson(json_doc_root, config_file);
            JsonObject root = json_doc_root.as<JsonObject>();

            DynamicJsonDocument json_doc_wifi(2048);
            deserializeJson(json_doc_wifi, root["wifi"].as<String>());
            JsonObject j_wifi = json_doc_wifi.as<JsonObject>();
            const char* wifi_ssid = j_wifi["ssid"];
            const char* wifi_password = j_wifi["pass"];
            const char* wifi_address = j_wifi["address"];
            int wifi_port = j_wifi["port"];

            DynamicJsonDocument json_doc_device(2048);
            deserializeJson(json_doc_device, root["device"].as<String>());
            JsonObject j_device = json_doc_device.as<JsonObject>();
            const char* device_sensor = j_device["sensor"];
            int device_interval = j_device["wake_interval"];

            strcpy(cfg.ssid, wifi_ssid);
            strcpy(cfg.password, wifi_password);
            strcpy(cfg.address, wifi_address);
            strcpy(cfg.sensor, device_sensor);
            cfg.port = wifi_port;
            cfg.interval = device_interval;
            serial_info_ln("Got data from config.");
        }
    }
    config_file.close();
}

void write_config()
{
    char json_output[2048];
    DynamicJsonDocument doc(2048);
    JsonObject wifi  = doc.createNestedObject("wifi");
    JsonObject device  = doc.createNestedObject("device");

    wifi["ssid"] = cfg.ssid;
    wifi["pass"] = cfg.password;
    wifi["address"] = cfg.address;
    wifi["port"] = cfg.port;

    device["sensor"] = cfg.sensor;
    device["wake_interval"] = cfg.interval;

    File config_file = SPIFFS.open(spiffs_config_file, "w");
    serializeJson(doc, config_file);
    config_file.close();

    blink(3);
    serial_info_ln("Wrote data to config.");
}

void clear_config()
{
    cfg.ssid[0] = (char)0;
    cfg.password[0] = (char)0;
    cfg.address[0] = (char)0;
    cfg.sensor[0] = (char)0;
    cfg.port = 0;
    cfg.interval = default_sleep_interval;
    write_config();
    serial_info_ln("Cleared config.");
}

/*
    Project Kawa Client | CAPTIVE PORTAL

    Date        2021.07.11
    Author      Philip Delorenzo
    License     None
    Repo        https://github.com/deloarts/project-kawa-devboard

*/

DNSServer dnsServer;
ESP8266WebServer server(80);

IPAddress ap_ip(hotspot_ip);
IPAddress ap_gw(hotspot_gateway);
IPAddress ap_mask(hotspot_subnet);

const char *ap_ssid = hotspot_ssid;
const char *ap_password = hotspot_password;

void handle_request()
{
    String request_uri = server.uri();
    serial_info_ln("Got request: " + request_uri);

    if (!(request_uri.endsWith(".css") || request_uri.endsWith(".js") || request_uri.endsWith(".ico")))
    {
        serial_blank_ln("Redirecting to /index.html");
        request_uri = "/index.html";
    }
    if (SPIFFS.exists(request_uri)) 
    {
        File request_file = SPIFFS.open(request_uri, "r");
        server.streamFile(request_file, get_content_type(request_uri));
        request_file.close();
    }
    else 
    {
        server.send(404, "text/plain", "404: Not Found.");
    }
}

void handle_request_get()
{
    serial_info_ln("Got request to " + String(api_get_config));
    byte network_count = WiFi.scanNetworks();

    char json_output[2048];
    DynamicJsonDocument doc(2048);
    // settings json: settings can be altered
    JsonObject j_settings = doc.createNestedObject("settings");
    j_settings["location"] = cfg.location;
    j_settings["ssid"] = cfg.ssid;
    j_settings["password"] = cfg.password;
    j_settings["address"] = cfg.address;
    j_settings["port"] = cfg.port;
    j_settings["sensor"] = cfg.sensor;
    j_settings["interval"] = cfg.interval;

    // device-json: cannot be altered
    JsonObject j_device = doc.createNestedObject("device");
    JsonObject j_firmware = j_device.createNestedObject("firmware");
    j_firmware["name"] = "Firmware";
    j_firmware["value"] = firmware_version;
    JsonObject j_spiffs = j_device.createNestedObject("spiffs");
    j_spiffs["name"] = "Filesystem";
    j_spiffs["value"] = filesystem_version;
    JsonObject j_boardname = j_device.createNestedObject("boardname");
    j_boardname["name"] = "Board";
    j_boardname["value"] = board_name;
    JsonObject j_device_id = j_device.createNestedObject("device_id");
    j_device_id["name"] = "Device ID";
    j_device_id["value"] = get_device_id();
    JsonObject j_hostname = j_device.createNestedObject("hostname");
    j_hostname["name"] = "Hostname";
    j_hostname["value"] = get_hostname();
    JsonObject j_mac = j_device.createNestedObject("mac");
    j_mac["name"] = "MAC";
    j_mac["value"] = get_mac_address();
    JsonObject j_battery = j_device.createNestedObject("battery");
    j_battery["name"] = "Battery";
    j_battery["value"] = String(voltage.value) + String(voltage.unit);
    JsonObject j_mode = j_device.createNestedObject("mode");
    j_mode["name"] = "Mode";
    j_mode["value"] = software_state;
 
    // avail-json: cannot be altered
    JsonObject j_available = doc.createNestedObject("avail");

    JsonArray j_avail_ssid = j_available.createNestedArray("ssid");
    for (byte i=0; i<network_count; i++)
        j_avail_ssid.add(WiFi.SSID(i));

    JsonArray j_avail_sen = j_available.createNestedArray("sensor");
    j_avail_sen.add(type_na);
    j_avail_sen.add(type_bme280);
    j_avail_sen.add(type_ds18b20);

    JsonArray j_avail_int = j_available.createNestedArray("interval");
    j_avail_int.add(1);
    j_avail_int.add(5);
    j_avail_int.add(15);
    j_avail_int.add(30);
    j_avail_int.add(45);
    j_avail_int.add(60);

    serializeJson(doc, json_output);
    server.sendHeader("Access-Control-Allow-Methods", "GET");
    server.sendHeader("Access-Control-Allow-Headers", "Origin, Accept");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json_output);
    serial_info_ln("Completed request.");
}

void handle_request_post()
{
    serial_info_ln("Got request to " + String(api_post_config));

    if (!server.hasArg("plain"))
    {
        server.send(200, "text/plain", "ER");
    }
    else
    {
        String server_arguments = server.arg("plain");

        DynamicJsonDocument json_doc_root(2048);
        deserializeJson(json_doc_root, server_arguments);
        JsonObject j_root = json_doc_root.as<JsonObject>();

        DynamicJsonDocument json_doc_body(2048);
        deserializeJson(json_doc_body, j_root["body"].as<String>());
        JsonObject j_body = json_doc_body.as<JsonObject>();

        const char* json_location = j_body["location"];
        const char* json_ssid = j_body["ssid"];
        const char* json_password = j_body["password"];
        const char* json_address = j_body["address"];
        const char* json_sensor = j_body["sensor"];
        int json_interval = j_body["interval"];
        int json_port = j_body["port"];

        strcpy(cfg.location, json_location);
        strcpy(cfg.ssid, json_ssid);
        strcpy(cfg.password, json_password);
        strcpy(cfg.address, json_address);
        strcpy(cfg.sensor, json_sensor);
        cfg.port = json_port;
        cfg.interval = json_interval;

        serial_info_ln("Got new config:");
        serial_enum_ln("Location: " + String(cfg.location));
        serial_enum_ln("SSID:     " + String(cfg.ssid));
        serial_enum_ln("Password: " + String(cfg.password));
        serial_enum_ln("Server:   " + String(cfg.address));
        serial_enum_ln("Port:     " + String(cfg.port));
        serial_enum_ln("Sensor:   " + String(cfg.sensor));
        serial_enum_ln("Interval: " + String(cfg.interval));

        delay(2000);
        server.send(200, "text/plain", "OK");
        write_config();
    }

    serial_info_ln("Completed request.");
}

void handle_request_restart()
{
    serial_info_ln("Got request to " + String(api_restart));
    server.send(200, "text/plain", "OK");
    serial_info_ln("Completed request.");

    delay(100);
    ESP.restart();
}

void handle_request_reset()
{
    serial_info_ln("Got request to " + String(api_reset));
    server.send(200, "text/plain", "OK");
    clear_config();
    serial_info_ln("Completed request.");
}

void launch_ap()
{
    boolean led_state = OFF;
    unsigned long blink_timer = 0;

    serial_info_ln("Configuring access point...");
    WiFi.softAPConfig(ap_ip, ap_gw, ap_mask);
    WiFi.softAP(ap_ssid, ap_password);
    delay(500);

    serial_info_ln("Setting up listeners...");
    server.on(api_get_config, handle_request_get);
    server.on(api_post_config, handle_request_post);
    server.on(api_restart, handle_request_restart);
    server.on(api_reset, handle_request_reset);
    server.onNotFound(handle_request);
    serial_info_ln("Starting http server...");
    server.begin();
    serial_info_ln("Starting dns server...");
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(hotspot_dns_port, "*", ap_ip);
    serial_info_ln("Starting mdns responder...");
    MDNS.begin(hotspot_hostname);
    MDNS.addService("http", "tcp", 80);
    MDNS.update();
    serial_info_ln("Server online.");

    while (true)
    {
        server.handleClient();
        dnsServer.processNextRequest();
        
        /*
            Indicate that the server is ready.
        */
        if (blink_timer < millis())
        {
            blink_timer = millis() + (led_state ? 2000 : 100);
            digitalWrite(out_led, led_state);
            led_state = !led_state;
        }
    }
}

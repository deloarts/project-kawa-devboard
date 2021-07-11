/*
    Project Kawa Client | UTILITY

    Date        2021.07.11
    Author      Philip Delorenzo
    License     None
    Repo        https://github.com/deloarts/project-kawa-devboard

*/

String get_content_type(String _filename)
{
    String content_type;
    if (_filename.endsWith(".html"))
        content_type = "text/html";
    else if (_filename.endsWith(".css"))
        content_type = "text/css";
    else if (_filename.endsWith(".js"))
        content_type = "application/javascript";
    else if (_filename.endsWith(".ico"))
        content_type = "image/x-icon";
    else
        content_type = "text/plain";
    return content_type;
}

String get_device_id()
{
    byte mac[6];
    String buffer = "";

    WiFi.macAddress(mac);
    for (byte i = 0; i < 6; i++)
        buffer += String(int(mac[i]));
    return buffer;
}

String get_mac_address()
{
    byte mac[6];
    String buffer = "";

    WiFi.macAddress(mac);
    for (byte i = 0; i < 6; i++)
        i < 5 ? buffer += String(int(mac[i]), HEX) + ":" : buffer += String(int(mac[i]), HEX);
    return buffer;
}

String get_ip_address()
{
    IPAddress ip = WiFi.localIP();
    return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

String get_hostname()
{
    return String(device_hostname_prefix) + get_device_id();
}

int8_t get_quality_from_dbm(int8_t dbm)
{
    if(dbm <= -100)
        return 0;
    else if(dbm >= -50)
        return 100;
    else
        return 2 * (dbm + 100);
}

void blink(byte amount)
{
    for (byte i=0; i<amount; i++)
    {
        digitalWrite(out_led, ON);
        delay(100);
        digitalWrite(out_led, OFF);
        delay(100);
    }
}

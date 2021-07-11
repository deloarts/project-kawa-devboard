/*
    Project Kawa Client | GLOBALS

    Date        2021.07.11
    Author      Philip Delorenzo
    License     None
    Repo        https://github.com/deloarts/project-kawa-devboard

*/

struct config
{
    char ssid[100];
    char password[100];
    char address[100];
    char sensor[50];
    int port = 0;
    int interval = default_sleep_interval;   
} cfg;

struct device
{
    bool ok = false;
    float value;
    char unit[10];
} voltage;

struct sensor
{
    bool available = false;
    float value;
    char unit[10];
} temperature, pressure, humidity, altitude;

struct update
{
    bool update_needed = false;
    String new_version = "";
} firmware, filesystem;

bool sensor_ok;

unsigned long startup_time = millis();
unsigned long connection_time;

WiFiClient client;
HTTPClient http;

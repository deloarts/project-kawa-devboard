/*
    Project Kawa Client | POWER

    Date        2021.07.11
    Author      Philip Delorenzo
    License     None
    Repo        https://github.com/deloarts/project-kawa-devboard

*/

void get_voltage()
{   
    float value = ESP.getVcc() / 1000.0f;
    serial_blank_ln("Battery: " + String(value) + unit_voltage);

    voltage.ok = value < battery_too_low ? false : true;
    voltage.value = value;
    strcpy(voltage.unit, unit_voltage);
}

void enter_deep_sleep_max()
{
    serial_info_ln("Entering deep sleep as long as possible.");
    sensor_sleep();
    ESP.deepSleep(ESP.deepSleepMax());
    delay(100);
}

void enter_deep_sleep(int minutes)
{
    serial_info_ln("Entering deep sleep for " + String(minutes) + " minutes.");
    digitalWrite(out_led, OFF);
    sensor_sleep();
    ESP.deepSleep(minutes * 60 * 1e6);
    delay(100);
}

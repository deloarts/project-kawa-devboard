/*
    Project Kawa Client | SENSOR

    Date        2021.07.11
    Author      Philip Delorenzo
    License     None
    Repo        https://github.com/deloarts/project-kawa-devboard

*/

#include <Wire.h>
#include <OneWire.h>
#include <Adafruit_BME280.h>
#include <DallasTemperature.h>

OneWire owr(ONE_WIRE_BUS);
Adafruit_BME280 sensor_bme280;
DallasTemperature sensor_ds18b20(&owr);

bool init_sensor()
{
    serial_info_ln("Initializing sensor " + String(cfg.sensor) + "...");

    if (strcmp(cfg.sensor, type_bme280) == 0)
    {
        Wire.begin(i2c_sda, i2c_scl);
        if (sensor_bme280.begin(0x76, &Wire))
        {
            temperature.available = true;
            pressure.available = true;
            altitude.available = true;
            humidity.available = true;

            sensor_bme280.wakeup();
            serial_info_ln("Sensor ok.");
            return true;
        }
        else
        {
            serial_error_ln("Could not establish a connection to the sensor.");
            return false;
        }
    }
    else if (strcmp(cfg.sensor, type_ds18b20) == 0)
    {
        sensor_ds18b20.begin();
        if (sensor_ds18b20.getDeviceCount() == 0)
        {
            serial_error_ln("Could not establish a connection to the sensor, count was zero.");
            return false;
        }
        else
        {
            temperature.available = true;
            serial_info_ln("Sensor ok.");
            return true;
        }
    }
    else
    {
        serial_error_ln("Sensor not recognized.");
        return false;
    }
}

void sensor_sleep()
{
    if (sensor_ok)
    {
        if (strcmp(cfg.sensor, type_bme280) == 0)
        {
            /*
                Adafruit_BME280::sleep(void) is not in the original library and has
                been added. Therefore this modified library is located inside the
                assets folder. 
            */
            sensor_bme280.sleep();
        }
    }
}

void sensor_wake()
{
    if (sensor_ok)
    {
        if (strcmp(cfg.sensor, type_bme280) == 0)
        {
            /*
                Adafruit_BME280::wake(void) is not in the original library and has
                been added. Therefore this modified library is located inside the
                assets folder. 
            */
            sensor_bme280.wakeup();
        }
    }
}

void _get_temperature()
{
    float value;
    serial_enum("Temperature: ");

    if (temperature.available)
    {
        if (strcmp(cfg.sensor, type_bme280) == 0)
        {
            value = sensor_bme280.readTemperature();
        }
        else if (strcmp(cfg.sensor, type_ds18b20) == 0)
        {
            sensor_ds18b20.requestTemperatures();
            value = sensor_ds18b20.getTempCByIndex(0);
            if (value == DEVICE_DISCONNECTED_C)
                sensor_ok = false;
        }
        
        temperature.value = value;
        strcpy(temperature.unit, unit_temperature);

        serial_print_ln(String(value) + unit_temperature);
    }
    else
    {
        serial_print_ln("N/A");
    }
}

void _get_pressure()
{
    float value;
    serial_enum("Pressure:    ");

    if (pressure.available)
    {
        if (strcmp(cfg.sensor, type_bme280) == 0)
            value = sensor_bme280.readPressure() / 100.0F;
        
        pressure.value = value;
        strcpy(pressure.unit, unit_pressure);

        serial_print_ln(String(value) + unit_pressure);
    }
    else
    {
        serial_print_ln("N/A");
    }
}

void _get_altitude()
{
    float value;
    serial_enum("Altitude:    ");

    if (altitude.available)
    {
        if (strcmp(cfg.sensor, type_bme280) == 0)
            value = sensor_bme280.readAltitude(altitude_p0);
        
        altitude.value = value;
        strcpy(altitude.unit, unit_altitude);

        serial_print_ln(String(value) + unit_altitude);
    }
    else
    {
        serial_print_ln("N/A");
    }
}

void _get_humidity()
{
    float value;
    serial_enum("Humidity:    ");

    if (humidity.available)
    {
        if (strcmp(cfg.sensor, type_bme280) == 0)
            value = sensor_bme280.readHumidity();
        
        humidity.value = value;
        strcpy(humidity.unit, unit_humidity);

        serial_print_ln(String(value) + unit_humidity);
    }
    else
    {
        serial_print_ln("N/A");
    }
}

void get_sensor_data()
{
    if (sensor_ok)
    {
        serial_info_ln("Reading sensor...");
        _get_temperature();
        _get_pressure();
        _get_altitude();
        _get_humidity();
    }
}

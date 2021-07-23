/*
    Project Kawa Client | DEFINITIONS

    Date        2021.07.11
    Author      Philip Delorenzo
    License     None
    Repo        https://github.com/deloarts/project-kawa-devboard

*/

#define board_name "devboard"
#define firmware_version "0.0.0d"
#define filesystem_version "0.0.0"

/*
    If mode is set to DEBUG, don't forget to add the 'd' to the
    firmware-version when building the final binary.
*/
#define DEBUG
#ifdef DEBUG
    #define software_state "debug"

    #define serial_init(x) Serial.begin(x)

    #define serial_print(x) Serial.print(x)
    #define serial_info(x) Serial.print("[INFO]\t\t" + String(x))
    #define serial_warning(x) Serial.print("[WARNING]\t" + String(x))
    #define serial_error(x) Serial.print("[ERROR]\t\t" + String(x))
    #define serial_blank(x) Serial.print("\t\t\t" + String(x))
    #define serial_enum(x) Serial.print("\t\t\t - " + String(x))

    #define serial_print_ln(x) Serial.println(x)
    #define serial_info_ln(x) Serial.println("[INFO]\t\t" + String(x))
    #define serial_warning_ln(x) Serial.println("[WARNING]\t" + String(x))
    #define serial_error_ln(x) Serial.println("[ERROR]\t\t" + String(x))
    #define serial_blank_ln(x) Serial.println("\t\t\t" + String(x))
    #define serial_enum_ln(x) Serial.println("\t\t\t - " + String(x))

    #define serial_print_hex(x) Serial.print(x, 16)
    #define serial_print_hex_ln(x) Serial.println(x, 16)
#else
    #define software_state "production"

    #define serial_init(x)

    #define serial_print(x)
    #define serial_info(x)
    #define serial_warning(x)
    #define serial_error(x)
    #define serial_blank(x)
    #define serial_enum(x)

    #define serial_print_ln(x)
    #define serial_info_ln(x)
    #define serial_warning_ln(x)
    #define serial_error_ln(x)
    #define serial_blank_ln(x)
    #define serial_enum_ln(x)

    #define serial_print_hex(x)
    #define serial_print_hex_ln(x)
#endif

#define ON 0x0
#define OFF 0x1

#define i2c_address 0x76
#define i2c_sda 5
#define i2c_scl 4

#define btn_1 12
#define btn_2 13
// #define btn_3 14
#define btn_reset 16
#define btn_upload 0
#define out_led 2
#define ONE_WIRE_BUS 14

// Power & deep sleep
#define battery_too_low 3.1
#define default_sleep_interval 60 //minutes

// Units
#define unit_interval "s"
#define unit_connection "ms"
#define unit_voltage "V"
#define unit_temperature "°C"
#define unit_pressure "mBar"
#define unit_altitude "m"
#define unit_humidity "%"

// Sensor types
#define type_bme280 "BME280"
#define type_ds18b20 "DS18B20"
#define type_na "N/A"

// Sensor corrections
#define altitude_p0 1013.25

// Server API
#define api_post_data "/data"
#define api_get_update "/update"

// Internal API
#define api_get_config "/get"
#define api_post_config "/post"
#define api_restart "/restart"
#define api_reset "/reset"

#define max_retries_wifi 100
#define max_retries_post 5

#define device_hostname_prefix "kawa_"

#define hotspot_hostname "kawa"
#define hotspot_ssid "kawa setup"
#define hotspot_password ""
#define hotspot_ip 1,2,3,4
#define hotspot_gateway 1,2,3,4
#define hotspot_subnet 255,255,255,0
#define hotspot_dns_port 53

// Files
#define spiffs_config_file "config.json"

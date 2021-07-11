/*
    Project Kawa Client | UPDATE

    Date        2021.07.11
    Author      Philip Delorenzo
    License     None
    Repo        https://github.com/deloarts/project-kawa-devboard

*/

// void _callback_started()
// {
//     serial_print_ln("Callback: http-update started.");
// }

// void _callback_finished()
// {
//     serial_print_ln("Callback: http-update finished.");
// }

// void _callback_progress()
// {
//     serial_print_ln("Callback:  http-update process at %d of %d bytes...\n", cur, total);
// }

// void _callback_error()
// {
//     serial_print_ln("Callback:  http-update fatal error code %d\n", err);
// }

void _update_handler(bool update_firmware, String new_version)
{
    serial_info("Running update for ");

    String update_url = "http://";
    update_url.concat(String(cfg.address) + ":" + String(cfg.port));
    update_url.concat(String(api_get_update));

    // ESPhttpUpdate.onStart(_callback_started);
    // ESPhttpUpdate.onEnd(_callback_finished);
    // ESPhttpUpdate.onProgress(_callback_progress);
    // ESPhttpUpdate.onError(_callback_error);

    t_httpUpdate_return return_value;
    ESPhttpUpdate.setLedPin(out_led, ON);
    ESPhttpUpdate.rebootOnUpdate(false);

    if (update_firmware)
    {
        update_url.concat("?what=firmware");
        update_url.concat("&for=" + String(board_name));
        update_url.concat("&get=" + String(new_version));

        serial_print_ln("firmware:");
        serial_enum_ln("Current: " + String(firmware_version));
        serial_enum_ln("New:     " + String(new_version));
        serial_enum_ln("Source:  " + String(update_url));

        return_value = ESPhttpUpdate.update(client, update_url);
    }
    else
    {
        /*
            Filesystem update will overwrite WIFI settings.
            Needs fixing.
        */
        update_url.concat("?what=spiffs");
        update_url.concat("&for=" + String(board_name));
        update_url.concat("&get=" + String(new_version));

        serial_print_ln("SPIFFS:");
        serial_enum_ln("Current: " + String(filesystem_version));
        serial_enum_ln("New:     " + String(new_version));
        serial_enum_ln("Source:  " + String(update_url));

        return_value = ESPhttpUpdate.updateSpiffs(client, update_url);
    }

    switch (return_value)
    {
        default:
        case HTTP_UPDATE_FAILED:
            serial_error_ln("Update failed (" + String(ESPhttpUpdate.getLastError()) + "): " + ESPhttpUpdate.getLastErrorString().c_str());
            break;
        case HTTP_UPDATE_NO_UPDATES:
            serial_warning_ln("No update applied.");
            break;
        case HTTP_UPDATE_OK:
            serial_info_ln("Update ok");
            break;
    }

    serial_info_ln("Rebooting to apply new version...");
    // ESP.reset();
    ESP.restart();
}

void update_firmware(String new_version)
{
    _update_handler(true, new_version);
}

void update_filesystem(String new_version)
{
    _update_handler(false, new_version);
}

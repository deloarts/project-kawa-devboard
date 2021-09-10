#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
	Project Kawa Devboard Test Server

    Author:			Philip Delorenzo
    Created:		2021.07.11
    Licence:		None

    Only use this server for testing the clients
    requests and responses. Do not use this server
    in a production env.

    Binaries must be copied into the servers /bin
    directory.

"""

# Python standard modules
import os
import json
from pathlib import Path

# Pip install modules
from flask import Flask, request, Response, send_from_directory
from rich.console import Console

console = Console()
server = Flask(__name__)
root = str(Path(__file__).resolve().parent).replace(os.sep, '/')
filename = 'filesystem_devboard_0.0.0_1210711.bin'
value = {
    'status': 'ok',
    'update_firmware': False,
    'new_firmware': '0.0.0',
    'update_spiffs': False,
    'new_spiffs': '0.0.0'
}

@server.route('/data', methods=['POST'])
def request_post() -> Response:
    """
    Handles the controllers post request.
    Just prints the json in a pretty way.

    Returns:
        Response: Answers with a firmware-info-json.
    """
    console.log(f'post request from {request.remote_addr!r}:')
    payload = json.loads(request.data)
    console.log(payload)
    return Response(json.dumps(value), mimetype = 'application/json', status=201) 

@server.route('/update', methods=['GET', 'POST'])
def request_update() -> Response:
    """
    Handles the controllers firmware-update request.
    New firmware must be placed inside the /bin folder.

    Returns:
        Response: Answers with an octet-stream of the binary file.
    """
    console.log(f'post request from {request.remote_addr!r}:')

    # only let the controller update its firmware once
    value["update_firmware"] = False
    value["update_spiffs"] = False

    return send_from_directory(
        directory=root+'/bin',
        filename=filename,
        as_attachment=True,
        mimetype='application/octet-stream',
        attachment_filename='src.ino.bin')

if __name__ == '__main__':
    server.run(port=5000, host='0.0.0.0', debug=True, use_reloader=True, threaded=True)

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
This test verifies that logging via a UDP connection works as expected.
'''

import json
import socket
import threading
import time
import b2test_utils


HOST = "127.0.0.1"
PORT = 9999
EXPECTED_MESSAGES = [
    {
        "level": "INFO",
        "message": "This is a INFO message",
        "variables": {},
        "module": "",
        "package": "steering",
        "function": "send_basf2_messages",
        "proc": -1,
        "count": 0,
        "initialize": True,
    },
    {
        "level": "WARNING",
        "message": "And this is a WARNING message",
        "variables": {},
        "module": "",
        "package": "steering",
        "function": "send_basf2_messages",
        "proc": -1,
        "count": 1,
        "initialize": True,
    },
]

stop_server = threading.Event()
received_messages = []


def udp_server():
    """UDP server that collects messages."""
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as server_socket:
        server_socket.bind((HOST, PORT))
        server_socket.settimeout(0.5)
        print(f"[Server] Started listening on {HOST}:{PORT}")
        while not stop_server.is_set():
            try:
                data, addr = server_socket.recvfrom(1024)
            except socket.timeout:
                continue
            message = data.decode()
            print(f"[Server] Received from {addr}: {message}")
            received_messages.append(message)
    print(f'[Server] Stopped listening on {HOST}:{PORT}')


def send_basf2_messages():
    """Send basf2 log messages via UDP."""
    import basf2

    basf2.logging.add_udp(HOST, PORT)
    basf2.B2INFO("This is a INFO message")
    basf2.B2WARNING("And this is a WARNING message")


def remove_fields(message_dict, fields):
    """Return a copy of the dict without the specified fields."""
    return {k: v for k, v in message_dict.items() if k not in fields}


server_thread = threading.Thread(target=udp_server, daemon=True)
server_thread.start()

time.sleep(0.5)
b2test_utils.run_in_subprocess(target=send_basf2_messages)
time.sleep(0.5)

stop_server.set()
server_thread.join()

for received_message in received_messages:
    received_dict = json.loads(received_message)
    normalized_received = remove_fields(received_dict, ["file", "line", "timestamp"])
    matched = any(normalized_received == expected for expected in EXPECTED_MESSAGES)
    assert matched, f"Received message not in expected messages: {normalized_received}"

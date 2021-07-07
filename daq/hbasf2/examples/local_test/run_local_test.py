##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import os
from time import sleep

from hlt.clean_execution import CleanBasf2Execution
from zmq_daq.example_support import get_sockets
from zmq_daq.utils import get_monitor_table, show_monitoring, write_monitoring

import yaml
import argparse


def yaml_settings(yaml_file):
    with open(yaml_file, "r") as f:
        return yaml.load(f)


def command_load(args):
    execution = CleanBasf2Execution()

    for app in args.settings_file["programs"]:
        execution.start(app)

    try:
        execution.wait()
    except KeyboardInterrupt:
        pass


def command_monitor(args):
    # Create and connect all needed sockets
    sockets = get_sockets(args.settings_file)

    # When no additional things are requested, just show the table once and exit
    if not args.watch and not args.dat:
        df = get_monitor_table(sockets, show_detail=args.show_detail)
        show_monitoring(df)
        exit()

    # Else we go into a main loop
    if args.dat:
        f = open(args.dat, "wb")
    try:
        while True:
            if args.watch:
                os.system("clear")

            df = get_monitor_table(sockets, show_detail=args.show_detail)
            if args.watch:
                show_monitoring(df)

            if args.dat:
                write_monitoring(df, f)

            sleep(0.5)
    finally:
        if args.dat:
            f.close()


def command_start(args):
    sockets = get_sockets(args.settings_file)
    for socket in sockets.values():
        socket.send_multipart([b"n", b"", b""])


def command_input_file(args):
    execution = CleanBasf2Execution()

    input_port = args.settings_file["input"]
    remaining_args = []
    if args.raw:
        remaining_args += ["--raw"]
    if args.repeat:
        remaining_args += ["--repeat"]
    execution.start(["b2hlt_file2socket", args.data_file, str(input_port)] + remaining_args)

    try:
        execution.wait()
    except KeyboardInterrupt:
        pass


def command_stop(args):
    sockets = get_sockets(args.settings_file)
    sockets["distributor"].send_multipart([b"l", b"", b""])

    if args.all:
        for socket in sockets.keys():
            if socket == "distributor":
                continue

            sockets[socket].send_multipart([b"l", b"", b""])


def command_terminate(args):
    sockets = get_sockets(args.settings_file)
    sockets["distributor"].send_multipart([b"x", b"", b""])

    if args.all:
        for socket in sockets.keys():
            if socket == "distributor":
                continue

            sockets[socket].send_multipart([b"x", b"", b""])


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="""Try out different modes for the ZMQ HLT.
    By choosing the settings_file, the different modes can be chosen.
    Together with the settings file, a command needs to be chosen.
    Typically, this will be "load" in one console and "input_file" in a different console.
    You can then monitor the process either with the "monitor" command
    or by using "b2hlt_monitor.py".
    """)

    parser.add_argument("settings_file", type=yaml_settings,
                        help="Which config file to load for the test")

    subparsers = parser.add_subparsers(help="Choose the command", dest="command")
    subparsers.required = True

    subparser = subparsers.add_parser("load")
    subparser.set_defaults(func=command_load)

    subparser = subparsers.add_parser("monitor")
    subparser.add_argument(
        "--show-detail",
        action="store_true",
        help="By default, some information are omitted. With this flag, all details are shown (can be very long output).")
    subparser.add_argument("--watch", action="store_true", help="Enter watch mode, where the script is called every 1 second.")
    subparser.add_argument("--dat", help="Write out the results of the monitoring periodically into a dat file. " +
                           "If not combined with --watch, the output will not be shown.")
    subparser.set_defaults(func=command_monitor)

    subparser = subparsers.add_parser("send_stop")
    subparser.add_argument("--all", action="store_true", help="Send the signal to all processes")
    subparser.set_defaults(func=command_stop)

    subparser = subparsers.add_parser("send_terminate")
    subparser.add_argument("--all", action="store_true", help="Send the signal to all processes")
    subparser.set_defaults(func=command_terminate)

    subparser = subparsers.add_parser("send_start")
    subparser.set_defaults(func=command_start)

    subparser = subparsers.add_parser("input_file")
    subparser.add_argument("data_file", help="Which data file to send")
    subparser.add_argument("--raw", help="send and receive raw data instead of event buffers", action="store_true")
    subparser.add_argument("--repeat", help="repeat after the file is finished", action="store_true")
    subparser.set_defaults(func=command_input_file)

    args = parser.parse_args()
    args.func(args)

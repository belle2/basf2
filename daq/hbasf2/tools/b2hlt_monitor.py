#!/usr/bin/env python3

# ************************************************************************#
# BASF2 (Belle Analysis Framework 2)                                      #
# Copyright(C) 2019 - Belle II Collaboration                              #
#                                                                         #
# Author: The Belle II Collaboration                                      #
# Contributors: Nils Braun                                                #
#                                                                         #
# This software is provided "as is" without any warranty.                 #
# ************************************************************************#

import zmq
from argparse import ArgumentParser
from zmq_daq.utils import get_monitor_table, normalize_addresses, write_monitoring, show_monitoring

from time import sleep

if __name__ == '__main__':
    parser = ArgumentParser(description="Monitor running ZMQ HLT applications with the given addresses.")
    parser.add_argument(
        "addresses",
        nargs='+',
        help="Monitor the given addresses. " +
        "Valid input formats are 'tcp://<host>:<port>', '<host>:<port>' or just '<port>' in which case localhost is assumed.")
    parser.add_argument(
        "--start",
        action="store_true",
        help="Additional to monitoring, also send out a START signal to all monitored processes.")
    parser.add_argument(
        "--stop",
        action="store_true",
        help="Additional to monitoring, also send out a STOP signal to all monitored processes.")
    parser.add_argument(
        "--abort",
        action="store_true",
        help="Send out a TERMINATE signal to all monitored processes and do not monitor afterwards.")

    parser.add_argument(
        "--show-detail",
        action="store_true",
        help="By default, some information are omitted. With this flag, all details are shown (can be very long output).")
    parser.add_argument("--watch", action="store_true", help="Enter watch mode, where the script is called every 1 second.")
    parser.add_argument("--dat", help="Write out the results of the monitoring periodically into a dat file. " +
                                      "If not combined with --watch, the output will not be shown.")

    args = parser.parse_args()

    addresses = normalize_addresses(args.addresses)

    # Create and connect all needed sockets
    ctx = zmq.Context()
    ctx.setsockopt(zmq.LINGER, 0)
    sockets = {address: ctx.socket(zmq.DEALER) for address in addresses}

    for address, socket in sockets.items():
        socket.connect(address)

    # Now send out signals if requested
    for socket in sockets.values():
        if args.start:
            socket.send_multipart([b"n", b"", b""])
        if args.stop:
            socket.send_multipart([b"l", b"", b""])
        if args.abort:
            socket.send_multipart([b"x", b"", b""])
            exit()

    try:
        # When no additional things are requested, just show the table once and exit
        if not args.watch and not args.dat:
            df = get_monitor_table(sockets, show_detail=args.show_detail)
            show_monitoring(df)
            exit()

        # Else we go into a main loop
        if args.dat:
            f = open(args.dat, "wb")

        while True:
            df = get_monitor_table(sockets, show_detail=args.show_detail)
            if args.watch:
                show_monitoring(df, clear=True)

            if args.dat:
                write_monitoring(df, f)

            sleep(0.5)
    except KeyboardInterrupt:
        pass
    finally:
        if args.dat:
            f.close()

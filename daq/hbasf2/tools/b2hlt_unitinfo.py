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
from argparse import ArgumentParser, FileType
from zmq_daq.utils import normalize_addresses, get_monitor_table

import os
from time import sleep
from collections import defaultdict


def get_overview(df):
    import pandas as pd
    tmp = pd.Series({tuple(key.split(".")): value for key, value in df.items()}).unstack(0).T

    df_to_show = pd.DataFrame(columns=tmp.index).T

    df_to_show["dead workers"] = tmp["input"]["dead_workers"]
    # this actually only exists for one, so we will the other with empty data
    df_to_show["registered workers"] = tmp["input"]["registered_workers"].fillna(0) + tmp["output"]["registered_workers"].fillna(0)
    df_to_show["ready queue size"] = tmp["output"]["ready_queue_size"]
    df_to_show["raw socket state"] = tmp["input"]["socket_state"].fillna("") + tmp["output"]["socket_state"].fillna("")

    # average between input and output
    df_to_show["data size"] = (tmp["input"]["data_size"].fillna(0) + tmp["output"]["data_size"].fillna(0))/2
    df_to_show["event rate"] = (tmp["input"]["event_rate"].fillna(0) + tmp["output"]["event_rate"].fillna(0))/2

    df_to_show = df_to_show.T.fillna("")

    worker_information = defaultdict(lambda: defaultdict(int))

    def add_information(col, grouped_df, prefix):
        if "[" not in col or not col.endswith("]"):
            return

        key, process_identifier = col[:-1].split("[")
        hostname, pid = process_identifier.split("_", 1)

        key = key.replace("_from", "").replace("_to", "")

        if key in ["total_number_messages", "hello_messages"]:
            return

        key = key.replace("received_", "").replace("sent_", "")
        key = key.replace("_", " ")

        worker_information[prefix + hostname][key] += grouped_df[col].dropna().iloc[0]

        if key == "data size":
            worker_information[prefix + hostname]["hosts"] += 1

    grouped_input = tmp["input"]
    for col in grouped_input.columns:
        add_information(col, grouped_input, "from ")

    grouped_output = tmp["output"]
    for col in grouped_output.columns:
        add_information(col, grouped_output, "to ")

    worker_information = pd.DataFrame(worker_information).T

    worker_information["hosts"] = pd.to_numeric(worker_information["hosts"], errors="coerce", downcast="integer")
    worker_information["ready messages"] = pd.to_numeric(worker_information["ready messages"], errors="coerce", downcast="integer")
    worker_information["events"] = pd.to_numeric(worker_information["events"], errors="coerce", downcast="integer")
    worker_information["data size"] = worker_information["data size"] / worker_information["hosts"]

    worker_information = worker_information.fillna("")

    worker_information = worker_information.loc[sorted(worker_information.index, key=lambda x: x.split(" ")[::-1])]

    return df_to_show, worker_information


if __name__ == '__main__':
    parser = ArgumentParser(
        description="Shortcut to b2hlt_monitor.py to only show the most relevant information for the full HLT unit")
    parser.add_argument(
        "addresses",
        nargs='*',
        help="Monitor the given addresses. " +
        "Valid input formats are 'tcp://<host>:<port>', '<host>:<port>' or just '<port>' in which case localhost is assumed.",
        default=["tcp://hltin:7000", "tcp://hltout:7000"])
    parser.add_argument("--watch", action="store_true", help="Enter watch mode, where the script is called every 1 second.")

    args = parser.parse_args()

    addresses = normalize_addresses(args.addresses)

    # Create and connect all needed sockets
    ctx = zmq.Context()
    sockets = {address: ctx.socket(zmq.DEALER) for address in addresses}

    for address, socket in sockets.items():
        socket.connect(address)

    # When no additional things are requested, just show the table once and exit
    if not args.watch:
        df = get_monitor_table(sockets, show_detail=True)
        df_to_show, worker_information = get_overview(df)
        print(df_to_show)
        print("")
        print(worker_information)
        exit()

    # Else we go into a main loop
    while True:
        df = get_monitor_table(sockets, show_detail=True)
        df_to_show, worker_information = get_overview(df)

        os.system("clear")
        print(df_to_show)
        print("")
        print(worker_information)

        sleep(1.0)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script to download the contents of a global tag of the central database.

This allows to use the payloads as a local payload directory or use it as a
local database when running basf2.
"""

import sys
import os
import requests
import shutil
from urllib.parse import urljoin
from . import calculate_checksum, encode_name
from .cli_utils import ItemFilter
from basf2 import B2ERROR, B2WARNING, B2INFO, LogLevel, LogInfo, logging
from concurrent.futures import ThreadPoolExecutor


def download_payload(destination, db, payloadinfo):
    """
    Download a single payload and return a list of all iovs. If the functions
    returns None there was an error downloading.
    """
    payload = payloadinfo["payloadId"]
    module = payload["basf2Module"]["name"]
    revision = int(payload["revision"])
    checksum = payload["checksum"]

    url = payload["payloadUrl"]
    base = payload["baseUrl"]
    local_file = os.path.join(destination, os.path.basename(url))
    remote_file = urljoin(base + "/", url)

    iovlist = []
    for iov in payloadinfo["payloadIovs"]:
        iovlist.append([module, revision, iov["expStart"], iov["runStart"], iov["expEnd"], iov["runEnd"]])

    # check if existing
    if os.path.exists(local_file):
        if calculate_checksum(local_file) == checksum:
            # done, nothing else to do
            return iovlist
        else:
            B2WARNING("Checksum mismatch for %s, downloading again" % local_file)

    # download the file
    B2INFO("download %s" % local_file)
    with open(local_file, "wb") as out:
        file_req = db._session.get(remote_file, stream=True)
        if file_req.status_code != requests.codes.ok:
            B2ERROR("Error downloading {0}: {1}".format(file_req.url, file_req.status_code))
            return None
        shutil.copyfileobj(file_req.raw, out)

    # and check it
    if calculate_checksum(local_file) != checksum:
        B2ERROR("Checksum mismatch after download: %s" % local_file)
        return None

    return iovlist


def command_download(args, db=None):
    """
    Download a global tag from the database

    This command allows to download a global tag from the central database to be
    used locally, either als lockup directory for payloads or as a standalone
    local database if --create-dbfile is specified.
    """

    payloadfilter = ItemFilter(args)

    if db is None:
        args.add_argument("tag", metavar="TAGNAME", default="production",
                          help="global tag to download")
        args.add_argument("destination", nargs='?', metavar="DIR", default="centraldb",
                          help="directory to put the payloads into (default: %(default)s)")
        args.add_argument("-c", "--create-dbfile", default=False, action="store_true",
                          help="if given save information about all payloads in DIR/database.txt")
        payloadfilter.add_arguments("payloads")
        args.add_argument("-j", type=int, default=1, dest="nprocess",
                          help="Number of concurrent connections to use for file "
                          "download (default: %(default)s)")
        args.add_argument("--retries", type=int, default=3,
                          help="Number of retries on connection problems (default: "
                          "%(default)s)")
        return

    try:
        os.makedirs(args.destination, exist_ok=True)
    except OSError:
        B2ERROR("cannot create destination directory", file=sys.stderr)
        return 1

    if not payloadfilter.check_arguments():
        return 1

    # modify logging to remove the useless module: lines
    for level in LogLevel.values.values():
        logging.set_info(level, LogInfo.LEVEL | LogInfo.MESSAGE | LogInfo.TIMESTAMP)

    req = db.request("GET", "/globalTag/{}/globalTagPayloads".format(encode_name(args.tag)),
                     "Downloading list of payloads for {tag} tag{}".format(payloadfilter, **vars(args)))
    B2INFO("Done")

    download_list = []
    for payload in req.json():
        name = payload["payloadId"]["basf2Module"]["name"]
        if payloadfilter.check(name):
            download_list.append(payload)

    # parse the payload list and do the downloading
    dbfile = []
    failed = 0
    with ThreadPoolExecutor(max_workers=args.nprocess) as pool:
        for iovlist in pool.map(lambda x: download_payload(args.destination, db, x), download_list):
            if iovlist is None:
                failed += 1
                continue

            for iov in iovlist:
                dbfile.append("dbstore/{} {} {},{},{},{}\n".format(*iov))

    if args.create_dbfile:
        with open(os.path.join(args.destination, "database.txt"), "w") as txtfile:
            txtfile.writelines(sorted(dbfile))

    if failed > 0:
        B2ERROR("Some payloads could not be downloaded")
        return 1

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script to upload local database to ConditionsDB.

This script takes a local database file and will upload all payloads defined in
this file to the ConditionsDB and create iovs for each payload. It assumes that
all tags already exist.
"""

import os
from basf2 import B2FATAL, B2ERROR, B2WARNING, B2INFO, LogLevel, LogInfo, logging


def command_upload(args, db=None):
    """
    Upload a local database to the conditions database.

    This command allows uploading a local database which was created by basf2 to
    the central database. It assumes that the global tag already exists so
    please create it before if necessary using 'tag create'.

    The command requires the tagname to upload the payloads to and a
    database.txt containing the payloads and their iovs. One can supply a
    directory where to look for the payloads, by default they are assumed to be
    in the same directory as the database text file.
    """

    if db is None:
        args.add_argument("tag", metavar="TAGNAME",
                          help="Global tag to use for iov creation")
        args.add_argument("payloadsfile", metavar="PAYLOADSFILE",
                          help="Testing payload storage file containing list of iovs")
        args.add_argument("--normalize", action="store_true", default=False,
                          help="Normalize the payload root files before the upload")
        args.add_argument("-j", type=int, default=1, dest="nprocess",
                          help="Number of concurrent connections to use for database "
                          "connection (default: %(default)s)")
        args.add_argument("--retries", type=int, default=3,
                          help="Number of retries on connection problems (default: "
                          "%(default)s)")
        args.add_argument("--ignore-existing", action="store_true", default=False,
                          help="Don't check if payloads or iovs already exist in database. "
                          "Speeds up initialization as the list of existing payloads "
                          "doesn't need to be downloaded. Can be used on first upload "
                          "but the script cannot resume an upload if this option is given")
        return

    # modify logging to remove the useless module: lines
    for level in LogLevel.values.values():
        logging.set_info(level, LogInfo.LEVEL | LogInfo.MESSAGE | LogInfo.TIMESTAMP)

    # do the upload
    return 0 if db.upload(args.dbfile, args.tag, args.normalize, args.ignore_existing, args.nprocess) else 1

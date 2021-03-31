#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script to upload local database to ConditionsDB.

This script takes a local database file and will upload all payloads defined in
this file to the ConditionsDB and create iovs for each payload. It assumes that
all tags already exist.
"""

from basf2 import LogInfo, LogLevel, logging


def command_upload(args, db=None):
    """
    Upload a local database to the conditions database.

    This command allows uploading a local database which was created by basf2 to
    the central database. It assumes that the globaltag already exists so
    please create it before if necessary using 'tag create'.

    The command requires the tagname to upload the payloads to and a
    database.txt containing the payloads and their iovs. One can supply a
    directory where to look for the payloads, by default they are assumed to be
    in the same directory as the database text file.
    """

    if db is None:
        args.add_argument("tag", metavar="TAGNAME",
                          help="globaltag to use for iov creation")
        args.add_argument("payloadsfile", metavar="PAYLOADSFILE",
                          help="Testing payload storage file containing list of iovs")
        normalize_group = args.add_mutually_exclusive_group()
        normalize_group.add_argument('--normalize', dest="normalize", default=False, action="store_true",
                                     help="Normalize the payload files to have reproducible checksums. "
                                     "This option should only be used if the payload files were created "
                                     "with an older software version (before release-04)")
        normalize_group.add_argument('--normalize-name', type=str,
                                     help="Set the file name in the root file metadata to the given value. "
                                     "This implicitly enables ``--normalize`` and should only be used if "
                                     "the payload files were created with an older software version (before release-04)")
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
    normalize = args.normalize_name if args.normalize_name is not None else args.normalize
    return 0 if db.upload(args.payloadsfile, args.tag, normalize, args.ignore_existing, args.nprocess) else 1

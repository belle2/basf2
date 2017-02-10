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
from . import calculate_checksum
from concurrent.futures import ThreadPoolExecutor


class LocalDatabaseEntry:
    """Class to keep information about an entry in the local database file"""
    def __init__(self, line, basedir):
        """Create new entry from line in database file"""
        name, revision, iov = line.split()
        name = name.split("/")
        #: module name
        self.module = name[1]
        #: filename
        self.filename = os.path.join(basedir, "{package}_{module}_rev_{revision}.root".format(
            package="dbstore", module=self.module, revision=revision,
        ))
        iov = [int(e) for e in iov.split(",")]
        #: experiment/run of the first run
        self.firstRun = {"exp": iov[0], "run": iov[1]}
        #: experiment/run of the final run
        self.finalRun = {"exp": iov[2], "run": iov[3]}
        #: variable for checksum, calculated on first access
        self.__checksum = None
        #: object to uniquely identify this entry (payload + iov)
        self.__id = (self.module,) + tuple(iov)
        #: payload id, to be filled later
        self.payload = None
        #: iov id, to be filled later
        self.iov = None

    @property
    def checksum(self):
        """Return checksum, calculated on first access"""
        if self.__checksum is None:
            self.__checksum = calculate_checksum(self.filename)
        return self.__checksum

    def __repr__(self):
        """Convert to useful string representation"""
        return repr(self.__id + (self.filename,))

    def __eq__(self, other):
        """Compare to other entries, only consider package, module and iov for equality"""
        return self.__id == other.__id

    def __le__(self, other):
        """Compare to other entries, only consider package, module and iov for equality"""
        return self.__id <= other.__id

    def __lt__(self, other):
        """Compare to other entries, only consider package, module and iov for equality"""
        return self.__id < other.__id

    def __hash__(self):
        """Provide hash function to be able to create a set"""
        return hash(self.__id)


def command_upload(args, db=None):
    """
    Upload a local database to the conditions database.

    This command allows uploading a local database which was created by basf2 to
    the central database. It assumes that the global tag already exists.
    """

    if db is None:
        args.add_argument("tag", metavar="TAGNAME",
                          help="Global tag to use for iov creation")
        args.add_argument("dbfile", metavar="DATABASEFILE",
                          help="Database file containing local list of iovs")
        args.add_argument("dbdir", nargs="?", metavar="DATABASEDIR",
                          help="Directory for the payloads. Defaults to the directory "
                          "of the database file")
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

    # need at least one worker thread
    if args.nprocess <= 0:
        B2WARNING("-j must be larger than zero, ignoring")
        args.nprocess = 1

    B2INFO("Using %d simultaneous connections" % args.nprocess)

    # and get the id for the global tag
    tagId = db.get_globalTagInfo(args.tag)
    if tagId is None:
        return 1
    tagId = tagId["globalTagId"]

    # make sure the database file exists
    if not os.path.exists(args.dbfile):
        B2FATAL("Given database file does not exist")

    # and set the directory if not given on the command line
    if args.dbdir is None:
        args.dbdir = os.path.dirname(args.dbfile)

    # first create a list of payloads and a set of all experiments we encountered
    # also count the missing files
    entries = []
    missing = 0
    with open(args.dbfile) as dbfile:
        for line in dbfile:
            entry = LocalDatabaseEntry(line, args.dbdir)
            if not os.path.exists(entry.filename):
                B2ERROR("Cannot find payload file %s" % entry.filename)
                missing += 1
            entries.append(entry)

    # ok, we couldn't find all payload files, no need to continue
    if missing > 0:
        B2ERROR("Some payload files could not be found, exiting")
        return 1

    # now we could have more than one payload with the same iov so let's go over
    # it again and remove duplicates but keep the last one for each
    entries = sorted(set(reversed(entries)))

    existing_payloads = {}
    existing_iovs = {}
    # multithreading for the win ...
    with ThreadPoolExecutor(max_workers=args.nprocess) as pool:
        B2INFO("Downloading information about existing payloads and iovs...")
        # if we want to check for existing payloads/iovs we shedule the download of
        # the full payload list. And write a message as each completes
        if not args.ignore_existing:
            payloads_future = pool.submit(db.get_payloads, args.tag)
            payloads_future.add_done_callback(lambda x: B2INFO("got info on existing payloads"))
            iovs_future = pool.submit(db.get_iovs, args.tag)
            iovs_future.add_done_callback(lambda x: B2INFO("got info on existing iovs"))
            existing_payloads = payloads_future.result()
            existing_iovs = iovs_future.result()

    # Ok, so let's check all payloads and see if we need to upload, create iovs
    # or do nothing for each entry.

    errors = 0
    for entry in entries:
        # check for existing payload using module,checksum as key
        payload_key = (entry.module, entry.checksum)
        if payload_key in existing_payloads:
            entry.payload = existing_payloads[payload_key]
            B2INFO("%s already existing in database, skipping upload" % entry.filename)
        # check for existing iov using (payload id, first run id, final run id)
        iov_key = (entry.payload, entry.firstRun["exp"], entry.firstRun["run"], entry.finalRun["exp"], entry.finalRun["run"])
        if iov_key in existing_iovs:
            entry.iov = existing_iovs[iov_key]
            B2INFO("iov for %s already existing in database, skipping iov creation" % entry.filename)

    # if we have errors we refuse to continue
    if errors > 0:
        B2ERROR("Errors when checking contents of database file, exiting")
        return 1

    # OK, finally we can do the upload. So let's define a function which uploads
    # if needed and just call this function for all entries because
    # multithreading

    def upload(entry):
        """Helper function to handle uploading and iov creation in multiple threads"""
        # if the entry doesn't have a payload id then upload the file and obtain
        # the new payload id
        if entry.payload is None:
            B2INFO("Uploading %s" % entry)
            entry.payload = db.create_payload(entry.module, entry.filename, entry.checksum)
            print(entry.payload)

        # if upload was successful (or preexisting payload) but if we don't have an iov then create one
        if entry.payload is not None and entry.iov is None:
            B2INFO("Creating iov for %s" % entry)
            entry.iov = db.create_iov(tagId, entry.payload,
                                      entry.firstRun["exp"], entry.firstRun["run"],
                                      entry.finalRun["exp"], entry.finalRun["run"])
            print(repr(entry.iov))

        # return the modified entry
        return entry

    failed = 0
    # upload the payloads in multiple threads
    with ThreadPoolExecutor(max_workers=args.nprocess) as pool:
        for entry in pool.map(upload, entries):
            # and check if it was succesful
            if entry.iov is None:
                failed += 1

    if failed > 0:
        B2ERROR("Some payloads could not be uploaded")
        return 1

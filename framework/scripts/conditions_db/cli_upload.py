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
from collections import defaultdict


class LocalDatabaseEntry:
    """Class to keep information about an entry in the local database file"""
    def __init__(self, line, basedir):
        """Create new entry from line in database file"""
        try:
            name, revision, iov = line.split()
        except ValueError:
            raise ValueError("line must be of the form 'dbstore/<payloadname> <revision> "
                             "<firstExp>,<firstRun>,<finalExp>,<finalRun>'")
        try:
            revision = int(revision)
        except ValueError:
            raise ValueError("revision must be an integer")

        try:
            #: module name
            self.module = name.split("/")[1]
        except IndexError:
            raise ValueError("payload name must be of the form dbstore/<payloadname>")

        try:
            iov = [int(e) for e in iov.split(",")]
        except ValueError:
            raise ValueError("experiment and run numbers must be integers")

        if len(iov) != 4:
            raise ValueError("IoV needs to be four values (firstExp,firstRun,finalExp,finalRun)")

        #: filename
        self.filename = os.path.join(basedir, "dbstore_{module}_rev_{revision}.root".format(
            module=self.module, revision=revision,
        ))
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

    @property
    def iov_tuple(self):
        """Return a tuple with the valid exp,run range"""
        return self.firstRun['exp'], self.firstRun['run'], self.finalRun['exp'], self.finalRun['run']


def parse_database_file(filename, payload_dir=None, check_existing=True):
    """
    Parse a local database file

    This is the python equivalent of LocalDatabase::readDatabase implemented in
    python. Each line in in the file should be of the form

    "dbstore/{payloadname} {revision} {firstExp},{firstRun},{finalExp},{finalRun}"

    Comments can be started using "#", everything including this character to
    the end of the line will be ignored.

    Parameters:
      filename (str): filename of the local database file to parse
      payload_dir (str): directories where the payloads should be. In case of
            None the directory of the database file will be used.
      check_existing (bool): if True check if the payloads actually exist where
            they should be

    Returns:
        a list of LocalDatabaseEntry objects or None if there were any errors
    """

    # make sure the database file exists
    if not os.path.exists(filename):
        B2ERROR("Given database file does not exist")
        return None

    # set the directory if not given
    if payload_dir is None:
        payload_dir = os.path.dirname(filename)

    # remember the list of errors before start of the function to see if we
    # create any new ones
    old_error_count = logging.log_stats[LogLevel.ERROR]
    entries = []
    with open(filename) as dbfile:
        for lineno, line in enumerate(dbfile, 1):
            # ignore comments
            line = line.split("#", 1)[0].strip()
            # and empty lines
            if not line:
                continue
            # parse the line
            try:
                entry = LocalDatabaseEntry(line, payload_dir)
            except ValueError as e:
                B2ERROR("{filename}:{line} error: {error}".format(
                    filename=filename, line=lineno, error=e
                ))
                continue

            if check_existing and not os.path.exists(entry.filename):
                B2ERROR("{filename}:{line} error: cannot find payload file {missing}".format(
                    filename=filename, line=lineno, missing=entry.filename
                ))
                continue

            entries.append(entry)

    # ok, if we had any errors so far, exit
    if logging.log_stats[LogLevel.ERROR] > old_error_count:
        return None

    return entries


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

    # first create a list of payloads
    entries = parse_database_file(args.dbfile, args.dbdir)
    if entries is None:
        B2ERROR("Problems with database file, exiting")
        return 1

    if not entries:
        B2INFO(f"No payloads found in {args.dbfile}, exiting")
        return 0

    # time to get the id for the global tag
    tagId = db.get_globalTagInfo(args.tag)
    if tagId is None:
        return 1
    tagId = tagId["globalTagId"]

    # now we could have more than one payload with the same iov so let's go over
    # it again and remove duplicates but keep the last one for each
    entries = sorted(set(reversed(entries)))

    # so let's have a list of all payloads (name, checksum) as some payloads
    # might have multiple iovs. Each payload gets a list of all of those
    payloads = defaultdict(list)
    for e in entries:
        payloads[(e.module, e.checksum)].append(e)

    existing_payloads = {}
    existing_iovs = {}

    def upload_payload(item):
        """Upload a payload file if necessary but first check list of existing payloads"""
        key, entries = item
        if key in existing_payloads:
            B2INFO(f"{key[0]} (md5:{key[1]}) already existing in database, skipping.")
            payload_id = existing_payloads[key]
        else:
            entry = entries[0]
            payload_id = db.create_payload(entry.module, entry.filename, entry.checksum)
            if payload_id is None:
                return False

            B2INFO(f"Created new payload {entry.payload} for {entry.module} (md5:{entry.md5})")

        for entry in entries:
            entry.payload = payload_id

        return True

    def create_iov(entry):
        """Create an iov if necessary but first check the list of existing iovs"""
        if entry.payload is None:
            return None

        iov_key = (entry.payload,) + entry.iov_tuple
        if iov_key in existing_iovs:
            entry.iov = existing_iovs[iov_key]
            B2INFO(f"IoV {entry.iov_tuple} for {entry.module} (md5:{entry.checksum}) already existing in database, skipping.")
        else:
            entry.iov = db.create_iov(tagId, entry.payload, *entry.iov_tuple)
            if entry.iov is None:
                return False

            B2INFO(f"Created IoV {entry.iov_tuple} for {entry.module} (md5:{entry.checksum})")

        return True

    # multithreading for the win ...
    with ThreadPoolExecutor(max_workers=args.nprocess) as pool:
        B2INFO("Downloading information about existing payloads and iovs...")
        # if we want to check for existing payloads/iovs we shedule the download of
        # the full payload list. And write a message as each completes
        if not args.ignore_existing:
            payloads_future = pool.submit(db.check_payloads, payloads.keys())
            payloads_future.add_done_callback(lambda x: B2INFO("got info on existing payloads"))
            iovs_future = pool.submit(db.get_iovs, args.tag)
            iovs_future.add_done_callback(lambda x: B2INFO("got info on existing iovs"))
            existing_payloads = payloads_future.result()
            existing_iovs = iovs_future.result()

        failed_payloads = sum(0 if result else 1 for result in pool.map(upload_payload, payloads.items()))
        if failed_payloads > 0:
            B2ERROR(f"{failed_payloads} payloads could not be uploaded")

        failed_iovs = sum(0 if result else 1 for result in pool.map(create_iov, entries))
        if failed_iovs > 0:
            B2ERROR(f"{failed_iovs} IoVs could not be created")

        return failed_payloads + failed_iovs

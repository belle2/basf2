#!/usr/bin/env python3

"""
Script to upload local database to ConditionsDB.

This script takes a local database file and will upload all payloads defined in
this file to the ConditionsDB and create iovs for each payload. It assumes that
all tags already exist.
"""

import os
from basf2 import B2ERROR, LogLevel, logging
from conditions_db import file_checksum
from B2Tools.b2root import normalize_file


class TestingPayloadEntry:
    """Class to keep information about an entry in the testing payloads storage file"""

    def __init__(self, line, basedir):
        """Create new entry from line in testing payloads storage file"""
        try:
            name, revision, iov = line.split()
        except ValueError:
            raise ValueError("line must be of the form 'dbstore/<payloadname> <revision> "
                             "<firstExp>,<firstRun>,<finalExp>,<finalRun>'")
        try:
            #: revision stored in the file
            self.revision = int(revision)
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
        self.filename = os.path.join(basedir, f"dbstore_{self.module}_rev_{self.revision}.root")
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

    def normalize(self, name=None, root_version=61408):
        """Normalize the root file to have the same checksum for the same content"""
        normalize_file(self.filename, in_place=True, name=name, root_version=root_version)
        self.__checksum = file_checksum(self.filename)

    @property
    def checksum(self):
        """Return checksum, calculated on first access"""
        if self.__checksum is None:
            self.__checksum = file_checksum(self.filename)
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

    def iov_str(self):
        """String representation of IoV"""
        return f"{self.firstRun['exp']}/{self.firstRun['run']} - {self.finalRun['exp']}/{self.finalRun['run']}"


def parse_testing_payloads_file(filename, check_existing=True):
    """
    Parse a testing payload storage file

    This is the python equivalent of TestingPayloadStorage::read implemented in
    python. Each line in in the file should be of the form

    "dbstore/{payloadname} {revision} {firstExp},{firstRun},{finalExp},{finalRun}"

    Comments can be started using "#", everything including this character to
    the end of the line will be ignored.

    Parameters:
      filename (str): filename of the testing payload storage file to parse
      check_existing (bool): if True check if the payloads actually exist where
            they should be

    Returns:
        a list of TestingPayloadEntry objects or None if there were any errors
    """

    # make sure the testing payload storage file exists
    if not os.path.exists(filename):
        B2ERROR("Given database file does not exist")
        return None

    # set the directory
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
                entry = TestingPayloadEntry(line, payload_dir)
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

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script to download the contents of a globaltag of the central database.

This allows to use the payloads as a local payload directory or use it as a
local database when running basf2.
"""

import sys
import os
import requests
import shutil
import fnmatch
import re
import sqlite3
import functools
from urllib.parse import urljoin
from . import ConditionsDB, encode_name, file_checksum
from .cli_utils import ItemFilter
from basf2 import B2ERROR, B2WARNING, B2INFO, LogLevel, LogInfo, logging
from concurrent.futures import ThreadPoolExecutor


def check_payload(destination, payloadinfo):
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

    return (local_file, remote_file, checksum, iovlist)


def download_file(db, local_file, remote_file, checksum, iovlist=None):
    """Actually download the file"""
    # check if existing
    if os.path.exists(local_file):
        if file_checksum(local_file) == checksum:
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
    if file_checksum(local_file) != checksum:
        B2ERROR("Checksum mismatch after download: %s" % local_file)
        return None

    return iovlist


def download_payload(db, name, revision, baseUrl, payloadUrl, checksum, directory):
    """Download a payload given enough meta information"""
    remote = urljoin(baseUrl, payloadUrl)
    local = os.path.join(directory, checksum[:2], f"{name}_r{revision}.root")
    try:
        os.makedirs(os.path.dirname(local), exist_ok=True)
    except OSError as e:
        B2ERROR(f"Cannot download payload: {e}")
        return
    download_file(db, local, remote, checksum)


def get_tagnames(db, patterns, use_regex=False):
    """Return a list of tags matching all patterns"""
    all_tags = db.get_globalTags()
    final = set()
    for tag in patterns:
        if not use_regex:
            tagnames = fnmatch.filter(all_tags, tag)
        else:
            try:
                tagname_regex = re.compile(tag, re.IGNORECASE)
            except Exception as e:
                B2ERROR(f"--tag-regex: '{tag}' is not a valid regular expression: {e}")
                return False
            tagnames = (e for e in all_tags if tagname_regex.search(e))

        final |= set(tagnames)
    return list(final)


def command_download(args, db=None):
    """
    Download a globaltag from the database

    This command allows to download a globaltag from the central database to be
    used locally, either als lookup directory for payloads or as a standalone
    local database if --create-dbfile is specified.

    The command requires the TAGNAME to download and optionally an output
    directory which defaults to centraldb in the local working directory. It
    will check for existing payloads in the output directory and only download
    payloads which are not present or don't have the excpeted checksum.

    One can filter the payloads to be downloaded by payload name using the
    --filter, --exclude and --regex options.
    """

    payloadfilter = ItemFilter(args)

    if db is None:
        args.add_argument("tag", metavar="TAGNAME", default="production",
                          help="globaltag to download")
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
        group = args.add_mutually_exclusive_group()
        group.add_argument("--tag-pattern", default=False, action="store_true",
                           help="if given, all globaltags which match the shell-style "
                           "pattern TAGNAME will be downloaded: ``*`` stands for anything, "
                           "``?`` stands for a single character. "
                           "If -c is given as well the database files will be ``DIR/TAGNAME.txt``")
        group.add_argument("--tag-regex", default=False, action="store_true",
                           help="if given, all globaltags matching the regular "
                           "expression given by TAGNAME will be downloaded (see "
                           "https://docs.python.org/3/library/re.html). "
                           "If -c is given as well the database files will be ``DIR/TAGNAME.txt``")
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

    tagnames = [args.tag]

    if args.tag_pattern or args.tag_regex:
        tagnames = get_tagnames(db, tagnames, args.tag_regex)

    for tagname in sorted(tagnames):
        try:
            req = db.request("GET", "/globalTag/{}/globalTagPayloads".format(encode_name(tagname)),
                             "Downloading list of payloads for {} tag{}".format(tagname, payloadfilter))
        except ConditionsDB.RequestError as e:
            B2ERROR(str(e))
            continue

        download_list = {}
        for payload in req.json():
            name = payload["payloadId"]["basf2Module"]["name"]
            if payloadfilter.check(name):
                local_file, remote_file, checksum, iovlist = check_payload(args.destination, payload)
                if local_file in download_list:
                    download_list[local_file][-1] += iovlist
                else:
                    download_list[local_file] = [local_file, remote_file, checksum, iovlist]

        # do the downloading
        full_iovlist = []
        failed = 0
        with ThreadPoolExecutor(max_workers=args.nprocess) as pool:
            for iovlist in pool.map(lambda x: download_file(db, *x), download_list.values()):
                if iovlist is None:
                    failed += 1
                    continue

                full_iovlist += iovlist

        if args.create_dbfile:
            dbfile = []
            for iov in sorted(full_iovlist):
                dbfile.append("dbstore/{} {} {},{},{},{}\n".format(*iov))
            dbfilename = tagname if (args.tag_pattern or args.tag_regex) else "database"
            with open(os.path.join(args.destination, dbfilename + ".txt"), "w") as txtfile:
                txtfile.writelines(dbfile)

    if failed > 0:
        B2ERROR("{} out of {} payloads could not be downloaded".format(failed, len(download_list)))
        return 1


def command_offlinecopy(args, db=None):
    """
    Download one ore more payloads into a sqlite database for local use

    This command allows to download the information from one or more globaltags
    from the central database to be used locally.

    The command requires at least one tag name to download. It will check for
    existing payloads in the output directory and only download payloads which
    are not present or don't have the excpeted checksum.

    By default this script will create a local directory called ``conditions``
    which contains a ``metadata.sqlite`` with all the payload information and
    sub directories containing all the payload files.

    This can be changed by specifying a different name for the metadata file but
    the payloads will always be saved in the same directory as the sqlite file.
    """

    if db is None:
        args.add_argument("tag", nargs="+", metavar="TAGNAME", help="globaltag to download")
        args.add_argument("-o", "--dbfile", metavar="DATABASEFILE", default="conditions/metadata.sqlite",
                          help="Name of the database file to create")
        args.add_argument("--append", action="store_true", default=False,
                          help="Append to the existing database file if possible. "
                          "Otherwise the content in the database file will be overwritten")
        group = args.add_mutually_exclusive_group()
        group.add_argument("--no-download", action="store_true", default=False,
                           help="Don't download any payloads, just download the metadata information")
        group.add_argument("--only-download", action="store_true", default=False,
                           help="Assume the metadata file is already correct, just make sure all payloads are downloaded")
        args.add_argument("--ignore-missing", action="store_true", default=False,
                          help="Ignore missing globaltags and download all other tags")
        args.add_argument("-j", type=int, default=1, dest="nprocess",
                          help="Number of concurrent connections to use for file "
                          "download (default: %(default)s)")
        args.add_argument("--retries", type=int, default=3,
                          help="Number of retries on connection problems (default: "
                          "%(default)s)")
        group = args.add_mutually_exclusive_group()
        group.add_argument("--tag-pattern", default=False, action="store_true",
                           help="if given, all globaltags which match the shell-style "
                           "pattern TAGNAME will be downloaded: ``*`` stands for anything, "
                           "``?`` stands for a single character. ")
        group.add_argument("--tag-regex", default=False, action="store_true",
                           help="if given, all globaltags matching the regular "
                           "expression given by TAGNAME will be downloaded (see "
                           "https://docs.python.org/3/library/re.html). ")
        return

    if args.tag_regex or args.tag_pattern:
        args.tag = get_tagnames(db, args.tag, args.tag_regex)

    destination = os.path.relpath(os.path.dirname(os.path.abspath(args.dbfile)))
    try:
        os.makedirs(destination, exist_ok=True)
    except OSError as e:
        B2ERROR(f"cannot create output directory,  {e}")
        return 1

    if not args.tag:
        B2ERROR("No tags selected, cannot continue")
        return 1

    def get_taginfo(tagname):
        """return the important information about all our globaltags"""
        tag_info = db.get_globalTagInfo(tagname)
        if not tag_info:
            B2ERROR(f"Cannot find globaltag {tagname}")
            return None
        return tag_info['globalTagId'], tag_info['name'], tag_info['globalTagStatus']['name']

    tags = [get_taginfo(tagname) for tagname in args.tag]
    if not args.ignore_missing and None in tags:
        return 1
    # ok, remove tags that didn't exist ...
    tags = [e for e in tags if e is not None]

    # remember baseURLs and payloadNames in the database to add them with ids
    baseURLs = {}
    payloadNames = {}

    # let's try to open the database file
    try:
        database = sqlite3.connect(args.dbfile)
        # we we only download we don't need to fix the schema but should make sure there's actually something in it
        if args.only_download:
            cursor = database.execute("SELECT count(*) from full_payloads")
            payloads = cursor.fetchone()[0]
            print(payloads)
            if payloads == 0:
                return 0
        else:
            # but if we do and we don't append we need to drop the old tables
            if not args.append:
                database.executescript("""
                    DROP VIEW IF EXISTS iov_payloads;
                    DROP INDEX IF EXISTS iov_index;
                    DROP TABLE IF EXISTS iovs;
                    DROP TABLE IF EXISTS globaltags;
                    DROP VIEW IF EXISTS full_payloads;
                    DROP TABLE IF EXISTS payloads;
                    DROP TABLE IF EXISTS payloadNames;
                    DROP TABLE IF EXISTS baseurls;
                    """)
            # and then recreate the tables if missing
            database.executescript("""
                CREATE TABLE IF NOT EXISTS baseurls (
                    baseUrlId INTEGER PRIMARY KEY,
                    baseUrl text UNIQUE
                );

                CREATE TABLE IF NOT EXISTS payloadNames (
                    payloadNameId INTEGER PRIMARY KEY,
                    payloadName TEXT UNIQUE
                );

                CREATE TABLE IF NOT EXISTS payloads (
                    payloadId INTEGER PRIMARY KEY,
                    payloadNameId INTEGER NOT NULL,
                    revision INTEGER NOT NULL,
                    checksum text NOT NULL,
                    payloadUrl text DEFAULT NULL,
                    baseUrlId INTEGER NOT NULL,
                    CONSTRAINT name_rev UNIQUE (payloadNameId, revision),
                    FOREIGN KEY (payloadNameId) REFERENCES payloadNames (payloadNameId)
                        ON UPDATE CASCADE ON DELETE RESTRICT
                    FOREIGN KEY (baseUrlId) REFERENCES baseurls (baseUrlId)
                        ON UPDATE CASCADE ON DELETE RESTRICT
                );

                CREATE VIEW IF NOT EXISTS full_payloads AS
                    SELECT payloadId, payloadName, revision, checksum,
                      ifnull(payloadUrl, 'dbstore/' || payloadName || '/dbstore_' || payloadName || '_rev_' || revision || '.root')
                      as payloadUrl, baseUrl
                    FROM payloads NATURAL JOIN payloadNames NATURAL JOIN baseurls;

                CREATE TABLE IF NOT EXISTS globaltags (
                    globalTagId INTEGER PRIMARY KEY,
                    globalTagName text UNIQUE,
                    globalTagStatus text NOT NULL
                );

                CREATE TABLE IF NOT EXISTS iovs (
                    globalTagId INTEGER NOT NULL,
                    payloadId INTEGER NOT NULL,
                    firstExp INTEGER NOT NULL,
                    firstRun INTEGER NOT NULL,
                    finalExp INTEGER NOT NULL,
                    finalRun INTEGER NOT NULL,
                    FOREIGN KEY (globalTagId) REFERENCES globaltags (globalTagId)
                        ON UPDATE CASCADE ON DELETE CASCADE,
                    FOREIGN KEY (payloadId) REFERENCES payloads (payloadId)
                        ON UPDATE CASCADE ON DELETE RESTRICT
                );

                CREATE UNIQUE INDEX IF NOT EXISTS iov_index on iovs (
                    globalTagId, firstExp, firstRun, finalExp, finalRun, payloadId
                );

                CREATE VIEW IF NOT EXISTS iov_payloads AS
                    SELECT globalTagName, payloadName, revision, checksum,
                        firstExp, firstRun, finalExp, finalRun, payloadUrl, baseUrl
                    FROM globaltags NATURAL JOIN iovs NATURAL JOIN full_payloads;
                """)
            # and if we append we need to know whats in it so we have the correct ids
            if args.append:
                # prefill existing baseURLs/payloadNames
                baseURLs = {row[0]: row[1] for row in database.execute("SELECT baseUrl,baseUrlId from baseurls")}
                payloadNames = {row[0]: row[1] for row in database.execute("SELECT payloadName, payloadNameId from payloadNames")}

    except Exception as e:
        B2ERROR(f"Cannot open output file {args.dbfile}: {e}")
        return 1

    # we know the tags, we have a database file ... lets get the metadata
    with ThreadPoolExecutor(max_workers=args.nprocess) as pool:
        if not args.only_download:
            # loop over all tags with their iovs being downloaded in parallel
            for tag_id, tag_name, tag_state, iovs in pool.map(lambda x: x + (db.get_all_iovs(x[1]),), tags):
                B2INFO(f"Adding metadata for {tag_name} to {args.dbfile}")
                database.execute("INSERT OR REPLACE INTO globaltags VALUES (?,?,?)", (tag_id, tag_name, tag_state))

                if args.append:
                    # remove existing iovs ... we want to append globaltags but we want all globaltags to be correct
                    database.execute("DELETE from iovs WHERE globalTagId=?", (tag_id,))

                all_payloads = {}
                all_iovs = []
                for p in iovs:
                    if p.base_url not in baseURLs:
                        baseURLs[p.base_url] = len(baseURLs) + 1
                        database.execute("INSERT OR REPLACE INTO baseurls VALUES (?,?)", (baseURLs[p.base_url], p.base_url))
                    if p.name not in payloadNames:
                        payloadNames[p.name] = len(payloadNames) + 1
                        database.execute("INSERT OR REPLACE INTO payloadNames VALUES (?,?)", (payloadNames[p.name], p.name))
                    url = None
                    if p.payload_url.lstrip('/') != f"dbstore/{p.name}/dbstore_{p.name}_rev_{p.revision}.root":
                        url = p.payload_url
                    all_payloads[p.payload_id] = (p.payload_id, payloadNames[p.name], p.revision,
                                                  p.checksum, url, baseURLs[p.base_url])
                    all_iovs.append((tag_id, p.payload_id) + p.iov)

                database.executemany("INSERT OR IGNORE INTO payloads VALUES (?,?,?,?,?,?)", all_payloads.values())
                database.executemany("INSERT INTO iovs VALUES (?,?,?,?,?,?)", all_iovs)

            database.commit()
            database.execute("VACUUM")

        # and finally download all necessary payloads for this file
        if args.no_download:
            return 0

        # ok download all files mentioned in this database file ...
        downloader = functools.partial(download_payload, db, directory=destination)
        # we don't really care for the result, we just collect it to be sure there are no delayed exceptions
        list(pool.map(lambda x: downloader(*x),
                      database.execute("SELECT payloadName, revision, baseUrl, payloadUrl, checksum from full_payloads")))
        database.close()

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
import functools
import textwrap
from urllib.parse import urljoin
from . import ConditionsDB, encode_name, file_checksum
from .cli_utils import ItemFilter
from .local_metadata import LocalMetadataProvider
from basf2 import B2ERROR, B2WARNING, B2INFO, LogLevel, LogInfo, logging
from basf2.utils import get_terminal_width
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


def download_payload(db, payload, directory):
    """Download a payload given a PayloadInformation object"""
    remote = urljoin(payload.base_url, payload.payload_url)
    local = os.path.join(directory, payload.checksum[:2], f"{payload.name}_r{payload.revision}.root")
    try:
        os.makedirs(os.path.dirname(local), exist_ok=True)
    except OSError as e:
        B2ERROR(f"Cannot download payload: {e}")
        return None
    return download_file(db, local, remote, payload.checksum, iovlist=local)


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


def command_legacydownload(args, db=None):
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

    .. versionadded:: release-04-00-00

       This has been renamed from ``download`` and is kept for compatibility

    .. deprecated:: release-04-00-00

       Downloading a globaltag should be done in the new format creating sqlite
       database files
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

    failed = 0
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


def command_download(args, db=None):
    """
    Download one or more payloads into a sqlite database for local use

    This command allows to download the information from one or more globaltags
    from the central database to be used locally.

    The command requires at least one tag name to download. It will check for
    existing payloads in the output directory and only download payloads which
    are not present or don't have the excpeted checksum.

    By default this script will create a local directory called ``conditions/``
    which contains a ``metadata.sqlite`` with all the payload information of all
    selected globaltags and sub directories containing all the payload files.

    This can be changed by specifying a different name for the metadata file
    using the ``-o`` argument but the payloads will always be saved in sub
    directoies in the same directory as the sqlite file.

    .. versionchanged:: release-04-00-00

       Previously this command was primarily intended to download payloads for
       one globaltag and optionally create a text file with payload information
       as well as download all necessary file. This has been changed and will
       now create a sqlite file containing the payload metadata. If you need the
       old behavior please use the command ``b2conditionsdb legacydownload``
    """

    if db is None:
        args.add_argument("tag", nargs="*", metavar="TAGNAME", help="globaltag to download")
        args.add_argument("-o", "--dbfile", metavar="DATABASEFILE", default="conditions/metadata.sqlite",
                          help="Name of the database file to create (default: %(default)s)")
        args.add_argument("-f", "--force", action="store_true", default=False,
                          help="Don't ask permission if the output database file exists")
        args.add_argument("--append", action="store_true", default=False,
                          help="Append to the existing database file if possible. "
                          "Otherwise the content in the database file will be overwritten")
        group = args.add_mutually_exclusive_group()
        group.add_argument("--no-download", action="store_true", default=False,
                           help="Don't download any payloads, just fetch the metadata information")
        group.add_argument("--only-download", action="store_true", default=False,
                           help="Assume the metadata file is already filled, just make sure all payloads are downloaded")
        args.add_argument("--delete-extra-payloads", default=False, action="store_true",
                          help="if given this script will delete all extra files "
                          "that follow the payload naming convention ``AB/{name}_r{revision}.root`` "
                          "if they are not referenced in the database file.")
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

    # if we only download we need no tags, but otherwise check the tag list
    if not args.only_download:
        if args.tag_regex or args.tag_pattern:
            args.tag = get_tagnames(db, args.tag, args.tag_regex)

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

        # so lets get info on all our tags and check if soem are missing ...
        with ThreadPoolExecutor(max_workers=args.nprocess) as pool:
            tags = list(pool.map(get_taginfo, args.tag))

        if not args.ignore_missing and None in tags:
            return 1
        # ok, remove tags that didn't exist ... and print the final list
        tags = sorted((e for e in tags if e is not None), key=lambda tag: tag[1])
        taglist = ["Selected globaltags:"]
        taglist += textwrap.wrap(", ".join(tag[1] for tag in tags), width=get_terminal_width(),
                                 initial_indent=" "*4, subsequent_indent=" "*4)
        B2INFO('\n'.join(taglist))

    # ok, we either download something or need to modify the db file, make sure
    # the output directory exists ...
    destination = os.path.relpath(os.path.dirname(os.path.abspath(args.dbfile)))
    try:
        os.makedirs(destination, exist_ok=True)
    except OSError as e:
        B2ERROR(f"cannot create output directory,  {e}")
        return 1

    if not os.path.exists(args.dbfile):
        # no file? no append!
        args.append = False
    elif not args.force and not args.only_download:
        # but if it exists ask the user ...
        query = input(f"Database file {args.dbfile} exists, " + ("overwrite" if not args.append else "append") + " (y/n) [n]? ")
        if query.lower().strip() not in ['y', 'yes']:
            B2ERROR("Output file exists, cannot continue")
            return 1

    try:
        # if we only download we can open readonly
        mode = "read" if args.only_download else ("append" if args.append else "overwrite")
        database = LocalMetadataProvider(args.dbfile,  mode)
        # we we only download we don't need to fix the schema but should make sure there's actually something in it
        if args.only_download:
            if database.get_payload_count() == 0:
                return 0

    except Exception as e:
        B2ERROR(f"Cannot open output file {args.dbfile}: {e}")
        return 1

    # we know the tags, we have a database file ... lets get the metadata
    with ThreadPoolExecutor(max_workers=args.nprocess) as pool:
        if not args.only_download:
            # loop over all tags with their iovs being downloaded in parallel
            for tag_id, tag_name, tag_state, iovs in pool.map(lambda x: x + (db.get_all_iovs(x[1]),), tags):
                B2INFO(f"Adding metadata for {tag_name} to {args.dbfile}")
                database.add_globaltag(tag_id, tag_name, tag_state, iovs)

        # and finally download all necessary payloads for this file
        if args.no_download:
            return 0

        # make sure all the payloads referenced in the file are present
        downloader = functools.partial(download_payload, db, directory=destination)
        all_payloads = set(pool.map(downloader, database.get_payloads()))

        if args.delete_extra_payloads:
            existing_files = set()
            for dirname, subdirs, filenames in os.walk(destination):
                # only look in sub directories matching a hex substring
                subdirs[:] = (e for e in subdirs if re.match('[0-9a-f]{2}', e))
                # and don't check files in top dir
                if dirname == destination:
                    continue
                # and add all others
                for filename in filenames:
                    if not re.match(r"(.+)_r(\d+).root", filename):
                        continue
                    existing_files.add(os.path.join(dirname, filename))

            extra_files = existing_files - all_payloads
            B2INFO(f"Deleting {len(extra_files)} additional payload files")
            # delete all the files and consume the results to trigger any errors
            list(pool.map(os.remove, extra_files))

        return 1 if None in all_payloads else 0

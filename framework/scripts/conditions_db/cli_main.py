#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This script provides a command line interface to all the tasks related to the
Conditions database: manage global tags and iovs as well as upload new payloads
or download of existing payloads.

The usage of this program is similar to git: there are sub commands like for
example ``tag`` wich groups all actions related to the management of global
tags. All the available commands are listed below.
"""

# Creation of command line parser is done semi-automatically: it looks for
# functions which are called command_* and will take the * as sub command name
# and the docstring as documentation: the first line as brief explanation and
# the remainder as dull documentation. command_foo_bar will create subcommand
# bar inside command foo. The function will be called first with an instance of
# argument parser where the command line options required for that command
# should be added. If the command is run it will be called with the parsed
# arguments first and an instance to the database object as second argument.

# Some of the commands are separated out in separate modules named cli_*.py

import os
import re
import argparse
import textwrap
import json
import difflib
from urllib.parse import urljoin
import shutil
import pprint
from basf2 import B2ERROR, B2WARNING, B2INFO, LogLevel, LogInfo, logging, \
    LogPythonInterface
from basf2.utils import pretty_print_table
from terminal_utils import Pager
from dateutil.parser import parse as parse_date
from getpass import getuser
from . import ConditionsDB, enable_debugging, encode_name
from .cli_utils import ItemFilter, PayloadInformation
# the command_* functions are imported but not used so disable warning about
# this if pylama/pylint is used to check
from .cli_upload import command_upload  # noqa
from .cli_download import command_download  # noqa


def escape_ctrl_chars(name):
    """Remove ANSI control characters from strings"""
    # compile the regex on first use and remember it
    if not hasattr(escape_ctrl_chars, "_regex"):
        escape_ctrl_chars._regex = re.compile("[\x00-\x1f\x7f-\x9f]")

    # escape the control characters by putting theim in as \xFF
    def escape(match):
        return "\\x{:02x}".format(ord(match.group(0)))

    return escape_ctrl_chars._regex.sub(escape, name)


def command_tag(args, db=None):
    """
    List, show, create, modify or clone global tags.

    This command allows to list, show, create modify or clone global tags in the
    central database. If no other command is given it will list all tags as if
    "%(prog)s show" was given.
    """

    # no arguments to define, just a group command
    if db is not None:
        # normal call, in this case we just divert to list all tags
        command_tag_list(args, db)


def command_tag_list(args, db=None):
    """
    List all available global tags.

    This command allows to list all global tags, optionally limiting the output
    to ones matching a given search term. By default invalidated global tags
    will not be included in the list, to show them as well please add
    --with-invalid as option. Alternatively one can use --only-published to show
    only tags which have been published

    If the --regex option is supplied the searchterm will interpreted as a
    python regular expression where the case is ignored.
    """

    tagfilter = ItemFilter(args)
    if db is None:
        # db object is none means we should just define arguments
        args.add_argument("--detail", action="store_true", default=False,
                          help="show detailed information for all tags instead "
                          "of summary table")
        group = args.add_mutually_exclusive_group()
        group.add_argument("--with-invalid", action="store_true", default=False,
                           help="if given also invalidated tags will be shown")
        group.add_argument("--only-published", action="store_true", default=False,
                           help="if given only published tags will be shown")
        tagfilter.add_arguments("tags")
        return

    # see if the -f/-e/-r flags are ok
    if not tagfilter.check_arguments():
        return 1

    req = db.request("GET", "/globalTags", "Getting list of global tags{}".format(tagfilter))

    # now let's filter the tags
    taglist = []
    for item in req.json():
        if not tagfilter.check(item["name"]):
            continue
        tagstatus = item["globalTagStatus"]["name"]
        if not getattr(args, "with_invalid", False) and tagstatus == "INVALID":
            continue
        if getattr(args, "only_published", False) and tagstatus != "PUBLISHED":
            continue
        taglist.append(item)

    # done, sort by name
    taglist.sort(key=lambda x: x["name"])

    # and print, either detailed info for each tag or summary table at the end
    table = []
    with Pager("List of global tags{}{}".format(tagfilter, " (detailed)" if getattr(args, "detail", False) else ""), True):
        for item in taglist:
            if getattr(args, "detail", False):
                print_globaltag(db, item)
            else:
                table.append([
                    item["globalTagId"],
                    item["name"],
                    escape_ctrl_chars(item.get("description", "")),
                    item["globalTagType"]["name"],
                    item["globalTagStatus"]["name"],
                    item["payloadCount"],
                ])

        if not getattr(args, "detail", False):
            table.insert(0, ["id", "name", "description", "type", "status", "# payloads"])
            pretty_print_table(table, [-10, 0, "*", -10, -10, -10], min_flexible_width=32)


def print_globaltag(db, *tags):
    """ Print detailed global tag information for the given global tags side by side"""
    results = [["id"], ["name"], ["description"], ["type"], ["status"],
               ["# payloads"], ["created"], ["modified"], ["modified by"]]
    for info in tags:
        if info is None:
            continue

        if isinstance(info, str):
            try:
                req = db.request("GET", "/globalTag/{}".format(encode_name(info)),
                                 "Getting info for global tag {}".format(info))
            except ConditionsDB.RequestError as e:
                # ok, there's an error for this one, let's continue with the other
                # ones
                B2ERROR(str(e))
                continue

            info = req.json()

        created = parse_date(info["dtmIns"])
        modified = parse_date(info["dtmMod"])
        results[0].append(str(info["globalTagId"])),
        results[1].append(info["name"]),
        results[2].append(escape_ctrl_chars(info.get("description", "")))
        results[3].append(info["globalTagType"]["name"])
        results[4].append(info["globalTagStatus"]["name"]),
        results[5].append(info["payloadCount"]),
        results[6].append(created.astimezone(tz=None).strftime("%Y-%m-%d %H:%M:%S local time"))
        results[7].append(modified.astimezone(tz=None).strftime("%Y-%m-%d %H:%M:%S local time"))
        results[8].append(escape_ctrl_chars(info["modifiedBy"]))

    ntags = len(results[0]) - 1
    if ntags > 0:
        pretty_print_table(results, [11] + ['*']*ntags, True)
    return ntags


def command_tag_show(args, db=None):
    """
    Show details about global tags

    This command will show details for the given global tags like name,
    description and number of payloads.
    """

    # this one is a bit similar to command_tag_list but gets single tag
    # informations from the database and thus no need for filtering. It will
    # always show the detailed information

    if db is None:
        args.add_argument("tag", metavar="TAGNAME", nargs="+", help="global tags to show")
        return

    # we retrieved all we could, print them
    ntags = 0
    with Pager("Global tag Information", True):
        for tag in args.tag:
            ntags += print_globaltag(db, tag)

    # return the number of tags which could not get retrieved
    return len(args.tag) - ntags


def command_tag_create(args, db=None):
    """
    Create a new global tag

    This command creates a new global tag in the database with the given name
    and description. The name can only contain alpha-numeric characters and the
    charachters '+-_:'.
    """

    if db is None:
        args.add_argument("type", metavar="TYPE", help="type of the global tag to create, usually one of DEV or RELEASE")
        args.add_argument("tag", metavar="TAGNAME", help="name of the global tag to create")
        args.add_argument("description", metavar="DESCRIPTION", help="description of the global tag")
        args.add_argument("-u", "--user", metavar="USER", help="username who created the tag. "
                          "If not given we will try to supply a useful default")
        return

    # create tag info needed for creation
    info = {"name": args.tag, "description": args.description, "modifiedBy": args.user, "isDefault": False}
    # add user information if not given by command line
    if args.user is None:
        info["modifiedBy"] = os.environ.get("BELLE2_USER", getuser())

    typeinfo = db.get_globalTagType(args.type)
    if typeinfo is None:
        return 1

    req = db.request("POST", "/globalTag/{}".format(encode_name(typeinfo["name"])),
                     "Creating global tag {name}".format(**info),
                     json=info)
    B2INFO("Succesfully created global tag {name} (id={globalTagId})".format(**req.json()))


def command_tag_modify(args, db=None):
    """
    Modify a global tag by changing name or description

    This command allows to change the name or description of an existing global tag.
    You can supply any combination of -n,-d,-t and only the given values will be changed
    """
    if db is None:
        args.add_argument("tag", metavar="TAGNAME", help="global tag to modify")
        args.add_argument("-n", "--name", help="new name")
        args.add_argument("-d", "--description", help="new description")
        args.add_argument("-t", "--type", help="new type of the global tag")
        args.add_argument("-u", "--user", metavar="USER", help="username who created the tag. "
                          "If not given we will try to supply a useful default")
        return

    # first we need to get the old tag information
    req = db.request("GET", "/globalTag/{}".format(encode_name(args.tag)),
                     "Getting info for global tag {}".format(args.tag))

    # now we update the tag information
    info = req.json()
    old_name = info["name"]
    for key in ["name", "description"]:
        if getattr(args, key) is not None:
            info[key] = getattr(args, key)

    info["modifiedBy"] = os.environ.get("BELLE2_USER", os.getlogin()) if args.user is None else args.user

    if args.type is not None:
        # for the type we need to know which types are defined
        typeinfo = db.get_globalTagType(args.type)
        if typeinfo is None:
            return 1
        # seems so, ok modify the tag info
        info["globalTagType"] = typeinfo

    # and push the changed info to the server
    db.request("PUT", "/globalTag",
               "Modifying global tag {} (id={globalTagId})".format(old_name, **info),
               json=info)


def command_tag_clone(args, db=None):
    """
    Clone a given global tag including all IoVs

    This command allows to clone a given global tag with a new name but still
    containing all the IoVs defined in the original global tag.
    """

    if db is None:
        args.add_argument("tag", metavar="TAGNAME", help="global tag to be cloned")
        args.add_argument("name", metavar="NEWNAME", help="name of the cloned global tag")
        return

    # first we need to get the old tag information
    req = db.request("GET", "/globalTag/{}".format(encode_name(args.tag)),
                     "Getting info for global tag {}".format(args.tag))
    info = req.json()

    # now we clone the tag. id came from the database so no need for escape
    req = db.request("POST", "/globalTags/{globalTagId}".format(**info),
                     "Cloning global tag {name} (id={globalTagId})".format(**info))

    # it gets a stupid name "{ID}_copy_of_{name}" so we change it to something
    # nice like the user asked
    cloned_info = req.json()
    cloned_info["name"] = args.name
    # and push the changed info to the server
    db.request("PUT", "/globalTag", "Renaming global tag {name} (id={globalTagId})".format(**cloned_info),
               json=cloned_info)


def command_tag_publish(args, db):
    """
    Publish a global tag.

    This command ets the state of a global tag to PUBLISHED. This will make the
    tag immutable and no more modifications are possible. A confirmation dialog
    will be shown
    """
    if db is None:
        args.add_argument("tag", metavar="TAGNAME", help="global tag to be published")
        return

    name = input("ATTENTION: Publishing a tag cannot be undone.\n"
                 "If you are sure you want to publish it please enter the tag name again: ")
    if name != args.tag:
        B2ERROR("Names don't match, aborting")
        return 1

    db.request("PUT", "/globalTag/{}/PUBLISH".format(encode_name(args.tag)),
               "Publishing global tag {}".format(args.tag))


def command_tag_invalidate(args, db):
    """
    Invalidate a global tag.

    This command ets the state of a global tag to INVALID. This will disqualify
    this tag from being used in user analysis.  A confirmation dialog will be
    shown.
    """
    if db is None:
        args.add_argument("tag", metavar="TAGNAME", help="global tag to be invalidated")
        return

    name = input("ATTENTION: invalidating a tag cannot be undone.\n"
                 "If you are sure you want to invalidate it please enter the tag name again: ")
    if name != args.tag:
        B2ERROR("Names don't match, aborting")
        return 1

    db.request("PUT", "/globalTag/{}/INVALID".format(encode_name(args.tag)),
               "invalidateing global tag {}".format(args.tag))


def command_diff(args, db):
    """Compare two global tags

    This command allows to compare two global tags. It will show the changes in
    a format similar to a unified diff but by default it will not show any
    context, only the new or removed payloads. Added payloads are marked with a
    ``+`` in the first column, removed payloads with a ``-``

    If ``--full`` is given it will show all payloads, even the ones common to
    both global tags. The differences can be limited to a given run and
    limited to a set of payloads names using ``--filter`` or ``--exclude``. If
    the ``--regex`` option is supplied the searchterm will interpreted as a
    python regular expression where the case is ignored.

    .. versionchanged:: release-03-00-00
       modified output structure and added ``--human-readable``
    """
    iovfilter = ItemFilter(args)
    if db is None:
        args.add_argument("--full", default=False, action="store_true",
                          help="If given print all iovs, also those which are the same in both tags")
        args.add_argument("--run", type=int, nargs=2, metavar="N", help="exp and run numbers "
                          "to limit showing iovs to a ones present in a given run")
        args.add_argument("--human-readable", default=False, action="store_true",
                          help="If given the iovs will be written in a more human friendly format. "
                          "Also repeated payload names will be omitted to create a more readable listing.")

        args.add_argument("tagA", metavar="TAGNAME1", help="base for comparison")
        args.add_argument("tagB", metavar="TAGNAME2", help="tagname to compare")
        iovfilter.add_arguments("payloads")
        return

    # check arguments
    if not iovfilter.check_arguments():
        return 1

    def get_iovlist(tag):
        """Return a list of minimized iov informations as json encoded strings
        to be able to run the difflib difference finder on them"""
        if args.run is not None:
            msg = "Obtaining list of iovs for global tag {tag}, exp={exp}, run={run}{filter}".format(
                tag=tag, exp=args.run[0], run=args.run[1], filter=iovfilter)
            req = db.request("GET", "/iovPayloads", msg, params={'gtName': tag, 'expNumber': args.run[0],
                                                                 'runNumber': args.run[1]})
        else:
            msg = "Obtaining list of iovs for global tag {tag}{filter}".format(tag=tag, filter=iovfilter)
            req = db.request("GET", "/globalTag/{}/globalTagPayloads".format(encode_name(tag)), msg)

        payloads = []
        for item in req.json():
            payload = item["payload" if 'payload' in item else "payloadId"]
            if "payloadIov" in item:
                iovs = [item['payloadIov']]
            else:
                iovs = item['payloadIovs']

            if not iovfilter.check(payload['basf2Module']['name']):
                continue

            for iov in iovs:
                payloads.append(PayloadInformation(payload, iov))

        payloads.sort()
        return payloads

    with Pager("Differences between global tags {tagA} and {tagB}{}".format(iovfilter, tagA=args.tagA, tagB=args.tagB), True):
        print("Global tags to be compared:")
        ntags = print_globaltag(db, args.tagA, args.tagB)
        if ntags != 2:
            return 1
        print()
        listA = get_iovlist(args.tagA)
        listB = get_iovlist(args.tagB)

        B2INFO("Comparing contents ...")
        diff = difflib.SequenceMatcher(a=listA, b=listB)
        if args.human_readable:
            table = [["", "Name", "Rev.", "Iov"]]
            columns = [1, "+", -8, -36]
        else:
            table = [["", "Name", "Rev.", "First Exp", "First Run", "Final Exp", "Final Run"]]
            columns = [1, "+", -8, 6, 6, 6, 6]

        def add_payloads(opcode, payloads):
            """Add a list of payloads to the table, filling the first column with opcode"""
            for p in payloads:
                if args.human_readable:
                    table.append([opcode, p.name, p.revision, p.readable_iov()])
                else:
                    table.append([opcode, p.name, p.revision] + list(p.iov))

        for tag, i1, i2, j1, j2 in diff.get_opcodes():
            if tag == "equal":
                if not args.full:
                    continue
                add_payloads(" ", listB[j1:j2])
            if tag in ["delete", "replace"]:
                add_payloads("-", listA[i1:i2])
            if tag in ["insert", "replace"]:
                add_payloads("+", listB[j1:j2])

        if args.human_readable:
            # strip repeated names, revision, payloadid, to make it more readable
            last_code = None
            last_name = None
            last_rev = None
            for i in range(len(table)):
                cur_code, cur_name, cur_rev = table[i][:3]
                if last_code == cur_code:
                    if cur_name == last_name:
                        table[i][1] = ""
                        if cur_rev == last_rev:
                            table[i][2] = ""
                last_code, last_name, last_rev = cur_code, cur_name, cur_rev

        def color_row(row, widths, line):
            if not LogPythonInterface.terminal_supports_colors():
                return line
            begin = {'+': '\x1b[32m', '-': '\x1b[31m'}.get(row[0], "")
            end = '\x1b[0m'
            return begin + line + end

        # print the table but make sure the first column is empty except for
        # added/removed lines so that it can be copy-pasted into a diff syntax
        # highlighting area (say pull request description)
        print(f" Differences between {args.tagA} and {args.tagB}")
        pretty_print_table(table, columns, transform=color_row,
                           hline_formatter=lambda w: " " + (w-1)*'-')


def command_iov(args, db):
    """
    List all IoVs defined in a global tag, optionally limited to a run range

    This command lists all IoVs defined in a given global tag. The list can be
    limited to a given run and optionally searched using --filter or --exclude.
    If the --regex option is supplied the searchterm will interpreted as a
    python regular expression where the case is ignored.

    .. versionchanged:: release-03-00-00
       modified output structure and added ``--human-readable``
    """

    iovfilter = ItemFilter(args)

    if db is None:
        args.add_argument("tag", metavar="TAGNAME", help="global tag for which the the IoVs should be listed")
        args.add_argument("--run", type=int, nargs=2, metavar="N", help="exp and run numbers "
                          "to limit showing iovs to a ones present in a given run")
        args.add_argument("--detail", action="store_true", default=False,
                          help="if given show a detailed information for all "
                          "IoVs including details of the payloads")
        args.add_argument("--human-readable", default=False, action="store_true",
                          help="If given the iovs will be written in a more human friendly format. "
                          "Also repeated payload names will be omitted to create a more readable listing.")
        iovfilter.add_arguments("payloads")
        return

    # check arguments
    if not iovfilter.check_arguments():
        return 1

    if args.run is not None:
        msg = "Obtaining list of iovs for global tag {tag}, exp={exp}, run={run}{filter}".format(
            tag=args.tag, exp=args.run[0], run=args.run[1], filter=iovfilter)
        req = db.request("GET", "/iovPayloads", msg, params={'gtName': args.tag, 'expNumber': args.run[0],
                                                             'runNumber': args.run[1]})
    else:
        msg = "Obtaining list of iovs for global tag {tag}{filter}".format(tag=args.tag, filter=iovfilter)
        req = db.request("GET", "/globalTag/{}/globalTagPayloads".format(encode_name(args.tag)), msg)

    with Pager("List of IoVs{}{}".format(iovfilter, " (detailed)" if args.detail else ""), True):
        payloads = []
        for item in req.json():
            payload = item["payload" if 'payload' in item else "payloadId"]
            if "payloadIov" in item:
                iovs = [item['payloadIov']]
            else:
                iovs = item['payloadIovs']

            if not iovfilter.check(payload['basf2Module']['name']):
                continue

            for iov in iovs:
                if args.detail:
                    # detailed mode, show a table with all information for each
                    # iov
                    iov_created = parse_date(iov["dtmIns"])
                    iov_modified = parse_date(iov["dtmMod"])
                    payload_created = parse_date(payload["dtmIns"])
                    payload_modified = parse_date(payload["dtmMod"])
                    result = [
                        ["IoV Id", str(iov["payloadIovId"])],
                        ["first experiment", iov["expStart"]],
                        ["first run", iov["runStart"]],
                        ["final experiment", iov["expEnd"]],
                        ["final run", iov["runEnd"]],
                        ["IoV created", iov_created.astimezone(tz=None).strftime("%Y-%m-%d %H:%M:%S local time")],
                        ["IoV modified", iov_modified.astimezone(tz=None).strftime("%Y-%m-%d %H:%M:%S local time")],
                        ["IoV modified by", iov["modifiedBy"]],
                        ["payload Id", str(payload["payloadId"])],
                        ["name", payload["basf2Module"]["name"]],
                        ["revision", payload["revision"]],
                        ["checksum", payload["checksum"]],
                        ["payloadUrl", payload["payloadUrl"]],
                        ["baseUrl", payload.get("baseUrl", "")],
                        # print created and modified timestamps in local time zone
                        ["payload created", payload_created.astimezone(tz=None).strftime("%Y-%m-%d %H:%M:%S local time")],
                        ["payload modified", payload_modified.astimezone(tz=None).strftime("%Y-%m-%d %H:%M:%S local time")],
                        ["payload modified by", escape_ctrl_chars(payload["modifiedBy"])],
                    ]
                    print()
                    pretty_print_table(result, [-40, '*'], True)
                else:
                    payloads.append(PayloadInformation(payload, iov))

        if not args.detail:
            payloads.sort()
            if args.human_readable:
                table = [["Name", "Rev", "IoV", "IovId", "PayloadId"]]
                table += [[p.name, p.revision, p.readable_iov(), p.iov_id, p.payload_id] for p in payloads]
                columns = ["+", -8, -32, 6, 9]
                # strip repeated names, revision, payloadid, to make it more readable
                last_name = None
                last_rev = None
                for i in range(len(table)):
                    cur_name, cur_rev = table[i][:2]
                    if cur_name == last_name:
                        table[i][0] = ""
                        if cur_rev == last_rev:
                            table[i][1] = ""
                            table[i][-1] = ""
                    last_name, last_rev = cur_name, cur_rev

            else:
                table = [["Name", "Rev", "First Exp", "First Run", "Final Exp", "Final Run", "IovId", "PayloadId"]]
                table += [[p.name, p.revision] + list(p.iov) + [p.iov_id, p.payload_id] for p in payloads]
                columns = ["+", -8, 6, 6, 6, 6, 6, 9]

            pretty_print_table(table, columns)


def command_dump(args, db):
    """
    Dump the content of a given payload

    .. versionadded:: release-03-00-00

    This command will dump the payload contents stored in a given payload. One
    can either specify the payloadId (from a previous output of
    ``b2conditionsdb iov``), the payload name and its revision in the central
    database, or directly specify a local database payload file.

    .. rubric:: Examples

    Dump the content of a previously downloaded payload file::

        $ b2conditionsdb dump -f localdb/dbstore_BeamParameters_rev_59449.root

    Dump the content of a payload by name and revision directly from the central database::

        $ b2conditionsdb dump -r BeamParameters 59449

    Or directly by payload id from a previous call to ``b2conditionsdb iov``::

        $ b2conditionsdb dump -i 59685

    .. rubric:: Usage

    Depending on whether you want to display a payload by its id in the
    database, its name and revision in the database or from a local file
    provide **one** of the arguments ``-i``, ``-r`` or ``-f``
    """
    if db is None:
        group = args.add_mutually_exclusive_group(required=True)
        group.add_argument("-i", "--id", metavar="PAYLOADID", help="payload id to dump")
        group.add_argument("-r", "--revision", metavar=("NAME", "REVISION"), nargs=2,
                           help="Name and revision of the payload to dump")
        group.add_argument("-f", "--file", metavar="FILENAME", help="Dump local payload file")
        args.add_argument("--show-typenames", default=False, action="store_true",
                          help="If given show the type names of all classes. "
                          "This makes output more crowded but can be helpful for complex objects.")
        args.add_argument("--show-streamerinfo", default=False, action="store_true",
                          help="If given show the StreamerInfo for the classes in the the payload file. "
                          "This can be helpful to find out which version of a payload object "
                          "is included and what are the members")

        return

    payload = None
    # local file, don't query database at all
    if args.file:
        filename = args.file
        if not os.path.isfile(filename):
            B2ERROR(f"Payloadfile {filename} could not be found")
            return 1

        match = re.match(r"^dbstore_(.*)_rev_(\d*).root$", os.path.basename(filename))
        if not match:
            B2ERROR("Filename doesn't follow database convention. Should be dbstore_${payloadname}_rev_${revision}.root")
            return 1
        name = match.group(1)
        revision = int(match.group(2))
        payloadId = "Unknown"
    else:
        # otherwise do just that: query the database for either payload id or
        # the name,revision
        if args.id:
            req = db.request("GET", f"/payload/{args.id}", "Getting payload info")
            payload = req.json()
        elif args.revision:
            name, rev = args.revision
            rev = int(rev)
            req = db.request("GET", f"/module/{name}/payloads", "Getting payload info")
            for p in req.json():
                if p["revision"] == rev:
                    payload = p
                    break
            else:
                B2ERROR(f"Cannot find payload {name} with revision {rev}")
                return 1

        name = payload["basf2Module"]["name"]
        url = payload["payloadUrl"]
        base = payload["baseUrl"]
        payloadId = payload["payloadId"]
        filename = urljoin(base + "/", url)
        revision = payload["revision"]
        del payload, base, url

    # late import of ROOT because of all the side effects
    from ROOT import TFile, TBufferJSON, cout

    # remote http opening or local file
    tfile = TFile.Open(filename)
    if not tfile.IsOpen():
        B2ERROR(f"Could not open payload file {filename}")
        return 1

    obj = tfile.Get(name)
    if not obj:
        B2ERROR(f"Could not find payload object in payload {filename}")
        return 1

    json_str = TBufferJSON.ConvertToJSON(obj)

    def drop_fbits(obj):
        """
        Drop some members from ROOT json output.

        We do not care about fBits, fUniqueID or the typename of sub classes,
        we assume users are only interested in the data stored in the member
        variables
        """
        obj.pop("fBits", None)
        obj.pop("fUniqueID", None)
        if not args.show_typenames:
            obj.pop("_typename", None)
        return obj

    with Pager(f"Contents of Payload {name}, revision {revision} (id {payloadId})", True):
        if args.show_streamerinfo:
            B2INFO("StreamerInfo of Payload {name}, revision {revision} (id {payloadId})")
            tfile.ShowStreamerInfo()
            # sadly this prints to std::cout or even stdout but doesn't flush ... so we have
            # to make sure std::cout is flushed before printing anything else
            cout.flush()
            # and add a newline
            print()

        B2INFO(f"Contents of Payload {name}, revision {revision} (id {payloadId})")
        # load the json as python object dropping some things we don't want to
        # print
        obj = json.loads(json_str.Data(), object_hook=drop_fbits)
        # print the object content using pretty print with a certain width
        pprint.pprint(obj, compact=True, width=shutil.get_terminal_size((80, 20))[0])

    tfile.Close()


class FullHelpAction(argparse._HelpAction):
    """Class to recusively show help for an ArgumentParser and all it's sub_parsers"""

    def print_subparsers(self, parser, prefix=""):
        """Print help message for given parser and call again for all sub parsers"""
        # retrieve subparsers from parser
        subparsers_actions = [
            action for action in parser._actions
            if isinstance(action, argparse._SubParsersAction)]
        # there will probably only be one subparser_action,
        # but better save than sorry
        for subparsers_action in subparsers_actions:
            # get all subparsers and print help
            for choice, subparser in subparsers_action.choices.items():
                print()
                print("Command '{}{}'".format(prefix, choice))
                print(subparser.format_help())

                self.print_subparsers(subparser, prefix="{}{} ".format(prefix, choice))

    def __call__(self, parser, namespace, values, option_string=None):
        """Show full help message"""
        # run in pager because amount of options will be looong
        with Pager("{} {}".format(parser.prog, option_string)):
            parser.print_help()
            self.print_subparsers(parser)
            parser.exit()


def get_argument_parser():
    """
    Build a parser with all arguments of all commands
    """
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--debugging", action="store_true",
                        help="Enable debugging of http traffic")
    parser.add_argument("--help-full", action=FullHelpAction,
                        help="show help message for all commands and exit")
    parser.add_argument("--base-url", default=None,
                        help="URI for the base of the REST API, if not given a list of default locations is tried")
    parser.add_argument("--http-auth", choices=["none", "basic", "digest"], default="basic",
                        help=argparse.SUPPRESS)
    parser.add_argument("--http-user", default="commonDBUser", help=argparse.SUPPRESS)
    parser.add_argument("--http-password", default="Eil9ohphoo2quot", help=argparse.SUPPRESS)
    parser.set_defaults(func=lambda x, y: parser.print_help())
    parsers = parser.add_subparsers(
        title="Top level commands",
        description="To get additional help, run '%(prog)s COMMAND --help'"
    )

    subparsers = {}
    # now we go through all the functions defined which start with command_
    for name, func in sorted(globals().items()):
        if not name.startswith("command_"):
            continue
        # we interpret command_foo_bar_baz as subcommand baz of subcommmand bar
        # of subcommand foo. So let's split this into all commands and remove
        # the command_
        parts = name.split('_')[1:]
        # now we need to get the subparsers instance for the parent commmand. if
        # the command is top level, e.g. foo, we just use parsers. Otherwise we
        # go look into the dict of subparsers for command chains.
        parent = parsers
        if(len(parts) > 1):
            parent_parser, parent = subparsers[tuple(parts[:-1])]
            # if we are the first subcommand to a given command we have to add
            # the subparsers. do that and add it back to the dict
            if parent is None:
                parent = parent_parser.add_subparsers(
                    title="sub commands",
                    description="To get additional help, run '%(prog)s COMMAND --help'"
                )
                # now we added a subparser let's also have a recursive
                # --help-full argument
                parent_parser.add_argument("--help-full", action=FullHelpAction,
                                           help="show help message for all commands and exit")
                subparsers[tuple(parts[:-1])][1] = parent
        # so we have our subparsers instance, now create argument parser for the
        # function. We use the first part of the function docstring as help text
        # and everything after the first empty line as description of the
        # command
        helptxt, description = textwrap.dedent(func.__doc__).split("\n\n", 1)
        command_parser = parent.add_parser(parts[-1], help=helptxt, description=description,
                                           formatter_class=argparse.RawDescriptionHelpFormatter)
        # now call the function with the parser as first argument and no
        # database instance. This let's them define their own arguments
        func(command_parser, None)
        # and set the function as default to be called for later
        command_parser.set_defaults(func=func)
        # also add it back to the list of subparsers
        subparsers[tuple(parts)] = [command_parser, None]

    return parser


def main():
    """
    Main function for the command line interface.

    it will automatically create an ArgumentParser including all functions which
    start with command_ in the global namespace as sub commmands. These
    functions should take the arguments as first argument and an instance of the
    ConditionsDB interface as second argument. If the db interface is None the
    first argument is an instance of argparse.ArgumentParser an in this case the
    function should just add all needed arguments to the argument parser and
    return.
    """

    # disable error summary
    logging.enable_summary(False)
    # log via python stdout to be able to capture
    logging.enable_python_logging = True
    # modify logging to remove the useless module: lines
    for level in LogLevel.values.values():
        logging.set_info(level, LogInfo.LEVEL | LogInfo.MESSAGE)

    parser = get_argument_parser()
    # done, all functions parsed. Create the database instance and call the
    # correct subfunction according to the selected argument
    args = parser.parse_args()

    if args.debugging:
        enable_debugging()

    # manage some common options for up and downloading. slightly hacky but
    # need to be given to ConditionsDB on construction so meh
    nprocess = getattr(args, "nprocess", 1)
    retries = getattr(args, "retries", 0)
    # need at least one worker thread
    if nprocess <= 0:
        B2WARNING("-j must be larger than zero, ignoring")
        args.nprocess = nprocess = 1

    conditions_db = ConditionsDB(args.base_url, nprocess, retries)

    if args.http_auth != "none":
        conditions_db.set_authentication(args.http_user, args.http_password, args.http_auth == "basic")

    try:
        return args.func(args, conditions_db)
    except ConditionsDB.RequestError as e:
        B2ERROR(str(e))
        return 1

#!/usr/bin/env python3

"""
This script provides a command line interface to all the tasks related to the
:ref:`Conditions database <conditionsdb_overview>`: manage globaltags and iovs as well as upload new payloads
or download of existing payloads.

The usage of this program is similar to git: there are sub commands like for
example ``tag`` which groups all actions related to the management of global
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
import sys
import argparse
import textwrap
import json
import difflib
import shutil
import pprint
import requests
from basf2 import B2ERROR, B2WARNING, B2INFO, LogLevel, LogInfo, logging, \
    LogPythonInterface
from basf2.utils import pretty_print_table
from terminal_utils import Pager
from dateutil.parser import parse as parse_date
from getpass import getuser
from conditions_db import ConditionsDB, enable_debugging, encode_name, PayloadInformation
from conditions_db.cli_utils import ItemFilter
# the command_* functions are imported but not used so disable warning about
# this if pylama/pylint is used to check
from conditions_db.cli_upload import command_upload  # noqa
from conditions_db.cli_download import command_download, command_legacydownload  # noqa
from conditions_db.cli_management import command_tag_merge, command_tag_runningupdate  # noqa


def escape_ctrl_chars(name):
    """Remove ANSI control characters from strings"""
    # compile the regex on first use and remember it
    if not hasattr(escape_ctrl_chars, "_regex"):
        escape_ctrl_chars._regex = re.compile("[\x00-\x1f\x7f-\x9f]")

    # escape the control characters by putting them in as \xFF
    def escape(match):
        if match.group(0).isspace():
            return match.group(0)
        return "\\x{:02x}".format(ord(match.group(0)))

    return escape_ctrl_chars._regex.sub(escape, name)


def command_tag(args, db=None):
    """
    List, show, create, modify or clone globaltags.

    This command allows to list, show, create modify or clone globaltags in the
    central database. If no other command is given it will list all tags as if
    "%(prog)s show" was given.
    """

    # no arguments to define, just a group command
    if db is not None:
        # normal call, in this case we just divert to list all tags
        command_tag_list(args, db)


def command_tag_list(args, db=None):
    """
    List all available globaltags.

    This command allows to list all globaltags, optionally limiting the output
    to ones matching a given search term. By default invalidated globaltags
    will not be included in the list, to show them as well please add
    --with-invalid as option. Alternatively one can use --only-published to show
    only tags which have been published

    If the --regex option is supplied the search term will be interpreted as a
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

    req = db.request("GET", "/globalTags", f"Getting list of globaltags{tagfilter}")

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
    with Pager("List of globaltags{}{}".format(tagfilter, " (detailed)" if getattr(args, "detail", False) else ""), True):
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
    """ Print detailed globaltag information for the given globaltags side by side"""
    results = [["id"], ["name"], ["description"], ["type"], ["status"],
               ["# payloads"], ["created"], ["modified"], ["modified by"]]
    for info in tags:
        if info is None:
            continue

        if isinstance(info, str):
            try:
                req = db.request("GET", "/globalTag/{}".format(encode_name(info)),
                                 f"Getting info for globaltag {info}")
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


def change_state(db, tag, state, force=False):
    """Change the state of a global tag

    If the new state is not revertible then ask for confirmation
    """
    state = state.upper()
    if state in ["INVALID", "PUBLISHED"] and not force:
        name = input(f"ATTENTION: Marking a tag as {state} cannot be undone.\n"
                     "If you are sure you want to continue it please enter the tag name again: ")
        if name != tag:
            B2ERROR("Names don't match, aborting")
            return 1

    db.request("PUT", f"/globalTag/{encode_name(tag)}/updateGlobalTagStatus/{state}",
               f"Changing globaltag state {tag} to {state}")


def command_tag_show(args, db=None):
    """
    Show details about globaltags

    This command will show details for the given globaltags like name,
    description and number of payloads.
    """

    # this one is a bit similar to command_tag_list but gets single tag
    # information from the database and thus no need for filtering. It will
    # always show the detailed information

    if db is None:
        args.add_argument("tag", metavar="TAGNAME", nargs="+", help="globaltags to show")
        return

    # we retrieved all we could, print them
    ntags = 0
    with Pager("globaltag Information", True):
        for tag in args.tag:
            ntags += print_globaltag(db, tag)

    # return the number of tags which could not get retrieved
    return len(args.tag) - ntags


def command_tag_create(args, db=None):
    """
    Create a new globaltag

    This command creates a new globaltag in the database with the given name
    and description. The name can only contain alpha-numeric characters and the
    characters '+-_:'.
    """

    if db is None:
        args.add_argument("type", metavar="TYPE", help="type of the globaltag to create, usually one of DEV or RELEASE")
        args.add_argument("tag", metavar="TAGNAME", help="name of the globaltag to create")
        args.add_argument("description", metavar="DESCRIPTION", help="description of the globaltag")
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
                     "Creating globaltag {name}".format(**info),
                     json=info)
    B2INFO("Successfully created globaltag {name} (id={globalTagId})".format(**req.json()))


def command_tag_modify(args, db=None):
    """
    Modify a globaltag by changing name or description

    This command allows to change the name or description of an existing globaltag.
    You can supply any combination of -n,-d,-t and only the given values will be changed
    """
    if db is None:
        args.add_argument("tag", metavar="TAGNAME", help="globaltag to modify")
        args.add_argument("-n", "--name", help="new name")
        args.add_argument("-d", "--description", help="new description")
        args.add_argument("-t", "--type", help="new type of the globaltag")
        args.add_argument("-u", "--user", metavar="USER", help="username who created the tag. "
                          "If not given we will try to supply a useful default")
        args.add_argument("-s", "--state", help="new globaltag state, see the command ``tag state`` for details")
        return

    # first we need to get the old tag information
    req = db.request("GET", "/globalTag/{}".format(encode_name(args.tag)),
                     f"Getting info for globaltag {args.tag}")

    # now we update the tag information
    info = req.json()
    old_name = info["name"]
    changed = False
    for key in ["name", "description"]:
        value = getattr(args, key)
        if value is not None and value != info[key]:
            info[key] = value
            changed = True

    info["modifiedBy"] = os.environ.get("BELLE2_USER", os.getlogin()) if args.user is None else args.user

    if args.type is not None:
        # for the type we need to know which types are defined
        typeinfo = db.get_globalTagType(args.type)
        if typeinfo is None:
            return 1
        # seems so, ok modify the tag info
        if info['gloalTagType'] != typeinfo:
            info["globalTagType"] = typeinfo
            changed = True

    # and push the changed info to the server
    if changed:
        db.request("PUT", "/globalTag",
                   "Modifying globaltag {} (id={globalTagId})".format(old_name, **info),
                   json=info)

    if args.state is not None:
        name = args.name if args.name is not None else old_name
        return change_state(db, name, args.state)


def command_tag_clone(args, db=None):
    """
    Clone a given globaltag including all IoVs

    This command allows to clone a given globaltag with a new name but still
    containing all the IoVs defined in the original globaltag.
    """

    if db is None:
        args.add_argument("tag", metavar="TAGNAME", help="globaltag to be cloned")
        args.add_argument("name", metavar="NEWNAME", help="name of the cloned globaltag")
        return

    # first we need to get the old tag information
    req = db.request("GET", "/globalTag/{}".format(encode_name(args.tag)),
                     f"Getting info for globaltag {args.tag}")
    info = req.json()

    # now we clone the tag. id came from the database so no need for escape
    req = db.request("POST", "/globalTags/{globalTagId}".format(**info),
                     "Cloning globaltag {name} (id={globalTagId})".format(**info))

    # it gets a stupid name "{ID}_copy_of_{name}" so we change it to something
    # nice like the user asked
    cloned_info = req.json()
    cloned_info["name"] = args.name
    # and push the changed info to the server
    db.request("PUT", "/globalTag", "Renaming globaltag {name} (id={globalTagId})".format(**cloned_info),
               json=cloned_info)


def command_tag_state(args, db):
    """
    Change the state of a globaltag.

    This command changes the state of a globaltag to the given value.

    Usually the valid states are

    OPEN
       Tag can be modified, payloads and iovs can be created and deleted. This
       is the default state for new or cloned globaltags and is not suitable
       for use in data analysis

       Can be transitioned to TESTING, RUNNING

    TESTING
       Tag cannot be modified and is suitable for testing but can be reopened

       Can be transitioned to VALIDATED, OPEN

    VALIDATED
       Tag cannot be modified and has been tested.

       Can be transitioned to PUBLISHED, OPEN

    PUBLISHED
       Tag cannot be modified and is suitable for user analysis

       Can only be transitioned to INVALID

    RUNNING
       Tag can only be modified by adding new runs, not modifying the payloads
       for existing runs.

    INVALID:
       Tag is invalid and should not be used for anything.

       This state is end of life for a globaltag and cannot be transitioned to
       any other state.

    .. versionadded:: release-04-00-00
    """
    if db is None:
        args.add_argument("tag", metavar="TAGNAME", help="globaltag to be changed")
        args.add_argument("state", metavar="STATE", help="new state for the globaltag")
        args.add_argument("--force", default=False, action="store_true", help=argparse.SUPPRESS)
        return

    return change_state(db, args.tag, args.state, args.force)


def command_tag_publish(args, db):
    """
    Publish a globaltag.

    This command sets the state of a globaltag to PUBLISHED. This will make the
    tag immutable and no more modifications are possible. A confirmation dialog
    will be shown

    .. deprecated:: release-04-00-00
       Use ``tag state $name PUBLISHED`` instead
    """
    if db is None:
        args.add_argument("tag", metavar="TAGNAME", help="globaltag to be published")
        return

    return change_state(db, args.tag, "PUBLISHED")


def command_tag_invalidate(args, db):
    """
    Invalidate a globaltag.

    This command sets the state of a globaltag to INVALID. This will disqualify
    this tag from being used in user analysis.  A confirmation dialog will be
    shown.

    .. deprecated:: release-04-00-00
       Use ``tag state $name INVALID`` instead
    """
    if db is None:
        args.add_argument("tag", metavar="TAGNAME", help="globaltag to be invalidated")
        return

    return change_state(db, args.tag, "INVALID")


def remove_repeated_values(table, columns, keep=None):
    """Strip repeated values from a table of values

    This function takes a table (a list of lists with all the same length) and
    removes values in certain columns if they are identical in consecutive rows.

    It does this in a dependent way only if the previous columns are identical
    will it continue stripping further columns. For example, given the table ::

        table = [
            ["A", "a"],
            ["B", "a"],
            ["B", "a"],
            ["B", "b"],
        ]

    If we want to remove duplicates in all columns in order it would look like this:

        >>> remove_repeated_values(table, [0,1])
        [
            ["A", "a"],
            ["B", "a"],
            [ "",  ""],
            [ "", "b"],
        ]

    But we can give selected columns to strip one after the other

        >>> remove_repeated_values(table, [1,0])
        [
            ["A", "a"],
            ["B",  ""],
            [ "",  ""],
            ["B", "b"],
        ]

    In addition, we might want to only strip some columns if previous columns
    were identical but keep the values of the previous column. For this one can
    supply ``keep``:

        >>> remove_repeated_values(table, [0,1,2], keep=[0])
        [
            ["A", "a"],
            ["B", "a"],
            ["B",  ""],
            ["B", "b"],
        ]

    Parameters:
        table (list(list(str))): 2d table of values
        columns (list(int)): indices of columns to consider in order
        keep (set(int)): indices of columns to not strip
    """
    last_values = [None] * len(columns)
    for row in table[1:]:
        current_values = [row[i] for i in columns]
        for i, curr, last in zip(columns, current_values, last_values):
            if curr != last:
                break

            if keep and i in keep:
                continue

            row[i] = ""

        last_values = current_values


def command_diff(args, db):
    """Compare two globaltags

    This command allows to compare two globaltags. It will show the changes in
    a format similar to a unified diff but by default it will not show any
    context, only the new or removed payloads. Added payloads are marked with a
    ``+`` in the first column, removed payloads with a ``-``

    If ``--full`` is given it will show all payloads, even the ones common to
    both globaltags. The differences can be limited to a given run and
    limited to a set of payloads names using ``--filter`` or ``--exclude``. If
    the ``--regex`` option is supplied the search term will be interpreted as a
    python regular expression where the case is ignored.

    .. versionchanged:: release-03-00-00
       modified output structure and added ``--human-readable``
    .. versionchanged:: after release-04-00-00
       added parameter ``--checksums`` and ``--show-ids``
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
        args.add_argument("--checksums", default=False, action="store_true",
                          help="If given don't show the revision number but the md5 checksum")
        args.add_argument("--show-ids", default=False, action="store_true",
                          help="If given also show the payload and iov ids for each iov")

        args.add_argument("tagA", metavar="TAGNAME1", help="base for comparison")
        args.add_argument("tagB", metavar="TAGNAME2", help="tagname to compare")
        iovfilter.add_arguments("payloads")
        return

    # check arguments
    if not iovfilter.check_arguments():
        return 1

    with Pager(f"Differences between globaltags {args.tagA} and {args.tagB}{iovfilter}", True):
        print("globaltags to be compared:")
        ntags = print_globaltag(db, args.tagA, args.tagB)
        if ntags != 2:
            return 1
        print()
        listA = [e for e in db.get_all_iovs(args.tagA, message=str(iovfilter)) if iovfilter.check(e.name)]
        listB = [e for e in db.get_all_iovs(args.tagB, message=str(iovfilter)) if iovfilter.check(e.name)]

        B2INFO("Comparing contents ...")
        diff = difflib.SequenceMatcher(a=listA, b=listB)
        table = [["", "Name", "Rev" if not args.checksums else "Checksum"]]
        columns = [1, "+", -8 if not args.checksums else -32]

        if args.human_readable:
            table[0] += ["Iov"]
            columns += [-36]
        else:
            table[0] += ["First Exp", "First Run", "Final Exp", "Final Run"]
            columns += [6, 6, 6, 6]

        if args.show_ids:
            table[0] += ["IovId", "PayloadId"]
            columns += [7, 9]

        def add_payloads(opcode, payloads):
            """Add a list of payloads to the table, filling the first column with opcode"""
            for p in payloads:
                row = [opcode, p.name, p.revision if not args.checksums else p.checksum]
                if args.human_readable:
                    row += [p.readable_iov()]
                else:
                    row += list(p.iov)

                if args.show_ids:
                    row += [p.iov_id, p.payload_id]
                table.append(row)

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
            # strip repeated names, revision, payloadid, to make it more readable.
            # this is dependent on the fact that the opcode is still the same but we
            # don't want to strip the opcode ...
            remove_repeated_values(table, [0, 1, 2] + ([-1] if args.show_ids else []), keep=[0])

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
    List all IoVs defined in a globaltag, optionally limited to a run range

    This command lists all IoVs defined in a given globaltag. The list can be
    limited to a given run and optionally searched using --filter or --exclude.
    If the --regex option is supplied the search term will be interpreted as a
    python regular expression where the case is ignored.

    .. versionchanged:: release-03-00-00
       modified output structure and added ``--human-readable``
    .. versionchanged:: after release-04-00-00
       added parameter ``--checksums`` and ``--show-ids``
    """

    iovfilter = ItemFilter(args)

    if db is None:
        args.add_argument("tag", metavar="TAGNAME", help="globaltag for which the the IoVs should be listed")
        args.add_argument("--run", type=int, nargs=2, metavar="N", help="exp and run numbers "
                          "to limit showing iovs to a ones present in a given run")
        args.add_argument("--detail", action="store_true", default=False,
                          help="if given show a detailed information for all "
                          "IoVs including details of the payloads")
        args.add_argument("--human-readable", default=False, action="store_true",
                          help="If given the iovs will be written in a more human friendly format. "
                          "Also repeated payload names will be omitted to create a more readable listing.")
        args.add_argument("--checksums", default=False, action="store_true",
                          help="If given don't show the revision number but the md5 checksum")
        args.add_argument("--show-ids", default=False, action="store_true",
                          help="If given also show the payload and iov ids for each iov")
        iovfilter.add_arguments("payloads")
        return

    # check arguments
    if not iovfilter.check_arguments():
        return 1

    if args.run is not None:
        msg = "Obtaining list of iovs for globaltag {tag}, exp={exp}, run={run}{filter}".format(
            tag=args.tag, exp=args.run[0], run=args.run[1], filter=iovfilter)
        req = db.request("GET", "/iovPayloads", msg, params={'gtName': args.tag, 'expNumber': args.run[0],
                                                             'runNumber': args.run[1]})
    else:
        msg = f"Obtaining list of iovs for globaltag {args.tag}{iovfilter}"
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
                    payloads.append(PayloadInformation.from_json(payload, iov))

        if not args.detail:
            def add_ids(table, columns, payloads):
                """Add the numerical ids to the table"""
                if args.show_ids:
                    table[0] += ["IovId", "PayloadId"]
                    columns += [7, 9]
                    for row, p in zip(table[1:], payloads):
                        row += [p.iov_id, p.payload_id]
            payloads.sort()
            if args.human_readable:
                table = [["Name", "Rev" if not args.checksums else "Checksum", "IoV"]]
                columns = ["+", -8 if not args.checksums else -32, -32]
                table += [[p.name, p.revision if not args.checksums else p.checksum, p.readable_iov()] for p in payloads]
                add_ids(table, columns, payloads)
                # strip repeated names, revision, payloadid, to make it more readable
                remove_repeated_values(table, columns=[0, 1] + ([-1] if args.show_ids else []))

            else:
                table = [["Name", "Rev" if not args.checksums else "Checksum", "First Exp", "First Run", "Final Exp", "Final Run"]]
                table += [[p.name, p.revision if not args.checksums else p.checksum] + list(p.iov) for p in payloads]
                columns = ["+", -8 if not args.checksums else -32, 6, 6, 6, 6]
                add_ids(table, columns, payloads)

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

    Dump the content of the payload by name which is valid in a given globaltag
    for a given experiment and run::

        $ b2conditionsdb dump -g BeamParameters master_2019-09-26 0 0

    Or directly by payload id from a previous call to ``b2conditionsdb iov``::

        $ b2conditionsdb dump -i 59685

    .. rubric:: Usage

    Depending on whether you want to display a payload by its id in the
    database, its name and revision in the database or from a local file
    provide **one** of the arguments ``-i``, ``-r``, ``-f`` or ``-g``

    .. versionchanged:: after release-04-00-00
       added argument ``-r`` to directly dump a payload valid for a given run
       in a given globaltag
    """
    if db is None:
        group = args.add_mutually_exclusive_group(required=True)
        choice = group.add_mutually_exclusive_group()
        choice.add_argument("-i", "--id", metavar="PAYLOADID", help="payload id to dump")
        choice.add_argument("-r", "--revision", metavar=("NAME", "REVISION"), nargs=2,
                            help="Name and revision of the payload to dump")
        choice.add_argument("-f", "--file", metavar="FILENAME", help="Dump local payload file")
        choice.add_argument("-g", "--valid", metavar=("NAME", "GLOBALTAG", "EXP", "RUN"), nargs=4,
                            help="Dump the payload valid for the given exp, run number in the given globaltag")
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

        match = re.match(r"^dbstore_(.*)_rev_(.*).root$", os.path.basename(filename))
        if not match:
            match = re.match(r"^(.*)_r(.*).root$", os.path.basename(filename))
        if not match:
            B2ERROR("Filename doesn't follow database convention.\n"
                    "Should be 'dbstore_${payloadname}_rev_${revision}.root' or '${payloadname}_r${revision.root}'")
            return 1
        name = match.group(1)
        revision = match.group(2)
        payloadId = "Unknown"
    else:
        # otherwise do just that: query the database for either payload id or
        # the name,revision
        if args.id:
            req = db.request("GET", f"/payload/{args.id}", "Getting payload info")
            payload = PayloadInformation.from_json(req.json())
            name = payload.name
        elif args.revision:
            name, rev = args.revision
            rev = int(rev)
            req = db.request("GET", f"/module/{encode_name(name)}/payloads", "Getting payload info")
            for p in req.json():
                if p["revision"] == rev:
                    payload = PayloadInformation.from_json(p)
                    break
            else:
                B2ERROR(f"Cannot find payload {name} with revision {rev}")
                return 1
        elif args.valid:
            name, globaltag, exp, run = args.valid
            payload = None
            for p in db.get_all_iovs(globaltag, exp, run, f", name={name}"):
                if p.name == name and (payload is None or p.revision > payload.revision):
                    payload = p

            if payload is None:
                B2ERROR(f"Cannot find payload {name} in globaltag {globaltag} for exp,run {exp},{run}")
                return 1

        filename = payload.url
        revision = payload.revision
        payloadId = payload.payload_id
        del payload

    # late import of ROOT because of all the side effects
    from ROOT import TFile, TBufferJSON, cout

    # remote http opening or local file
    tfile = TFile.Open(filename)
    json_str = None
    raw_contents = None
    if not tfile or not tfile.IsOpen():
        # could be a non-root payload file
        contents = db._session.get(filename, stream=True)
        if contents.status_code != requests.codes.ok:
            B2ERROR(f"Could not open payload file {filename}")
            return 1
        raw_contents = contents.raw.read().decode()
    else:
        obj = tfile.Get(name)
        if obj:
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
        if args.show_streamerinfo and tfile:
            B2INFO("StreamerInfo of Payload {name}, revision {revision} (id {payloadId})")
            tfile.ShowStreamerInfo()
            # sadly this prints to std::cout or even stdout but doesn't flush ... so we have
            # to make sure std::cout is flushed before printing anything else
            cout.flush()
            # and add a newline
            print()

        if json_str is not None:
            B2INFO(f"Contents of Payload {name}, revision {revision} (id {payloadId})")
            # load the json as python object dropping some things we don't want to
            # print
            obj = json.loads(json_str.Data(), object_hook=drop_fbits)
            # print the object content using pretty print with a certain width
            pprint.pprint(obj, compact=True, width=shutil.get_terminal_size((80, 20))[0])
        elif raw_contents:
            B2INFO(f"Raw contents of Payload {name}, revision {revision} (id {payloadId})")
            print(escape_ctrl_chars(raw_contents))
        elif tfile:
            B2INFO(f"ROOT contents of Payload {name}, revision {revision} (id {payloadId})")
            B2WARNING("The payload is a valid ROOT file but doesn't contain a payload object with the expected name. "
                      " Automated display of file contents are not possible, showing just entries in the ROOT file.")
            tfile.ls()


class FullHelpAction(argparse._HelpAction):
    """Class to recursively show help for an ArgumentParser and all it's sub_parsers"""

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
                print(f"Command '{prefix}{choice}'")
                print(subparser.format_help())

                self.print_subparsers(subparser, prefix=f"{prefix}{choice} ")

    def __call__(self, parser, namespace, values, option_string=None):
        """Show full help message"""
        # run in pager because amount of options will be looong
        with Pager(f"{parser.prog} {option_string}"):
            parser.print_help()
            self.print_subparsers(parser)
            parser.exit()


def get_argument_parser():
    """
    Build a parser with all arguments of all commands
    """
    # extra ArgumentParser with the global options just for reusability
    options = argparse.ArgumentParser(add_help=False)
    options.add_argument("--debugging", action="store_true",
                         help="Enable debugging of http traffic")
    options.add_argument("--help-full", action=FullHelpAction,
                         help="show help message for all commands and exit")
    options.add_argument("--base-url", default=None,
                         help="URI for the base of the REST API, if not given a list of default locations is tried")
    options.add_argument("--http-auth", choices=["none", "basic", "digest"], default="basic",
                         help=argparse.SUPPRESS)
    options.add_argument("--http-user", default="commonDBUser", help=argparse.SUPPRESS)
    options.add_argument("--http-password", default="Eil9ohphoo2quot", help=argparse.SUPPRESS)

    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter, parents=[options])
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
                subparsers[tuple(parts[:-1])][1] = parent
        # so we have our subparsers instance, now create argument parser for the
        # function. We use the first part of the function docstring as help text
        # and everything after the first empty line as description of the
        # command
        helptxt, description = textwrap.dedent(func.__doc__).split("\n\n", 1)
        command_parser = parent.add_parser(parts[-1], help=helptxt, add_help=True, description=description,
                                           parents=[options], formatter_class=argparse.RawDescriptionHelpFormatter)
        # now call the function with the parser as first argument and no
        # database instance. This let's them define their own arguments
        func(command_parser, None)
        # and set the function as default to be called for later
        command_parser.set_defaults(func=func)
        # also add it back to the list of subparsers
        subparsers[tuple(parts)] = [command_parser, None]

    return parser


def create_symlinks(base):
    """Create symlinks from base to all subcommands.

    e.g. if the base is ``b2conditionsdb`` then this command will create symlinks
    like ``b2conditionsdb-tag-show`` in the same directory

    When adding a new command to b2conditionsdb this function needs to be executed
    in the framework tools directory

    python3 -c 'from conditions_db import cli_main; cli_main.create_symlinks("b2conditionsdb")'
    """
    import os
    excluded = [
        ['tag']  # the tag command without subcommand is not very useful
    ]
    for name in sorted(globals().keys()):
        if not name.startswith("command_"):
            continue
        parts = name.split("_")[1:]
        if parts in excluded:
            continue
        dest = base + "-".join([""] + parts)

        try:
            os.remove(dest)
        except FileNotFoundError:
            pass
        print(f"create symlink {dest}")
        os.symlink(base, dest)


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

    # Ok, some people prefer `-` in the executable name for tab completion so lets
    # support that by just splitting the executable name
    sys.argv[0:1] = os.path.basename(sys.argv[0]).split('-')

    # parse argument definition for all sub commands
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

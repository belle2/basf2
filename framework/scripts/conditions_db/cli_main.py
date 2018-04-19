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
from basf2 import B2ERROR, B2WARNING, B2INFO, LogLevel, LogInfo, logging, \
    pretty_print_table, LogPythonInterface
from pager import Pager
from dateutil.parser import parse as parse_date
from . import ConditionsDB, enable_debugging, encode_name
from .cli_utils import ItemFilter
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
        info["modifiedBy"] = os.environ.get("BELLE2_USER", os.getlogin())

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
    """
    iovfilter = ItemFilter(args)
    if db is None:
        args.add_argument("--full", default=False, action="store_true",
                          help="If given print all iovs, also those which are the same in both tags")
        args.add_argument("--run", type=int, nargs=2, metavar="N", help="exp and run numbers "
                          "to limit showing iovs to a ones present in a given run")
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

        table = []
        for item in req.json():
            payload = item["payload" if 'payload' in item else "payloadId"]
            if "payloadIov" in item:
                iovs = [item['payloadIov']]
            else:
                iovs = item['payloadIovs']

            if not iovfilter.check(payload['basf2Module']['name']):
                continue

            for iov in iovs:
                table.append([
                        iov["expStart"], iov["runStart"], iov["expEnd"], iov["runEnd"],
                        payload['basf2Module']['name'], payload['revision'],
                        payload['payloadId'],
                    ])

        table.sort()
        return [json.dumps(e) for e in table]

    with Pager("Differences between global tags {tagA} and {tagB}{}".format(iovfilter, tagA=args.tagA, tagB=args.tagB), True):
        print("Global tags to be compared:")
        ntags = print_globaltag(db, args.tagA, args.tagB)
        if ntags != 2:
            return 1

        print()
        tableA = get_iovlist(args.tagA)
        tableB = get_iovlist(args.tagB)
        diff = [["", "First Exp", "First Run", "Final Exp", "Final Run", "Name", "Rev.", "PayloadId"]]
        B2INFO("Comparing contents ...")

        def color_row(row, widths, line):
            if not LogPythonInterface.terminal_supports_colors():
                return line
            begin = {'+': '\x1b[32m', '-': '\x1b[31m'}.get(row[0], "")
            end = '\x1b[0m'
            return begin + line + end

        for token in difflib.ndiff(tableA, tableB):
            if token.startswith("?") or (not args.full and token.startswith(" ")):
                continue
            row = json.loads(token[2:])
            diff.append([token[0]] + row)

        print(f"Differences betwen {args.tagA} and {args.tagB}")
        pretty_print_table(diff, [1, 6, 6, 6, 6, "+", -8, 10], transform=color_row)


def command_iov(args, db):
    """
    List all IoVs defined in a global tag, optionally limited to a run range

    This command lists all IoVs defined in a given global tag. The list can be
    limited to a given run and optionally searched using --filter or --exclude.
    If the --regex option is supplied the searchterm will interpreted as a
    python regular expression where the case is ignored.
    """

    iovfilter = ItemFilter(args)

    if db is None:
        args.add_argument("tag", metavar="TAGNAME", help="global tag for which the the IoVs should be listed")
        args.add_argument("--run", type=int, nargs=2, metavar="N", help="exp and run numbers "
                          "to limit showing iovs to a ones present in a given run")
        args.add_argument("--detail", action="store_true", default=False,
                          help="if given show a detailed information for all "
                          "IoVs including details of the payloads")
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
        table = []
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
                    table.append([
                        iov["expStart"], iov["runStart"], iov["expEnd"], iov["runEnd"],
                        payload['basf2Module']['name'], payload['revision'], iov['payloadIovId'],
                        payload['payloadId'],
                    ])

        if not args.detail:
            table.sort()
            table_header = ["First Exp", "First Run", "Final Exp", "Final Run", "Name", "Rev.", "IovId", "PayloadId"]
            table.insert(0, table_header)
            pretty_print_table(table, [6, 6, 6, 6, '+', -8, 6, 9])


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
    parser.add_argument("--base-url", default=ConditionsDB.BASE_URL,
                        help="URI for the base of the REST API (default: %(default)s)")
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

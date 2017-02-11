#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This script provides a command line interface to all the tasks related to the
Conditions database: manage global tags and iovs as well as upload new payloads
or download of existing payloads. Some of the commands are separated out in
separate modules named cli_*.py
"""

import os
import argparse
from basf2 import B2ERROR, B2WARNING, LogLevel, LogInfo, logging, pretty_print_table
from pager import Pager
from dateutil.parser import parse as parse_date
from . import ConditionsDB, enable_debugging
from .cli_utils import ItemFilter
# the command_* functions are imported but not used so disable warning about
# this if pylama/pylint is used to check
from .cli_upload import command_upload  # noqa
from .cli_download import command_download  # noqa


def command_tag(args, db=None):
    """
    List, show, modify or clone global tags.

    This command allows to list, show, modify or clone global tags in the
    central database. If not other command is given it will default to show all tags.
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

    The searchterm will interpreted as a python regular expression where the
    case is ignored if the --regex option is supplied.
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
    with Pager("List of global tags{}{}".format(tagfilter, " (detailed)" if getattr(args, "detail", False) else "")):
        for item in taglist:
            if getattr(args, "detail", False):
                print_globaltag(item)
            else:
                table.append([
                    item["globalTagId"],
                    item["name"],
                    item.get("description", ""),
                    item["globalTagType"]["name"],
                    item["globalTagStatus"]["name"],
                    item["payloadCount"],
                ])

        if not getattr(args, "detail", False):
            table.insert(0, ["id", "name", "description", "type", "status", "# payloads"])
            pretty_print_table(table, [-10, -30, "*", -10, -10, -10])


def print_globaltag(info):
    """ Print detailed global tag information for the given global tag"""
    created = parse_date(info["dtmIns"])
    modified = parse_date(info["dtmMod"])
    print()
    results = [
        ["id", str(info["globalTagId"])],
        ["name", info["name"]],
        ["description", info.get("description", "")],
        ["type", info["globalTagType"]["name"]],
        ["status", info["globalTagStatus"]["name"]],
        ["# payloads", info["payloadCount"]],
        # print created and modified timestamps in local time zone
        ["created", created.astimezone(tz=None).strftime("%Y-%m-%d %H:%M:%S local time")],
        ["modified", modified.astimezone(tz=None).strftime("%Y-%m-%d %H:%M:%S local time")],
        ["modified by", info["modifiedBy"]],
    ]
    pretty_print_table(results, [-40, '*'], True)


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

    objects = []
    for tag in args.tag:
        try:
            req = db.request("GET", "/globalTag/{}".format(tag),
                             "Getting info for global tag {}".format(tag))
        except ConditionsDB.RequestError as e:
            # ok, there's an error for this one, let's continue with the other
            # ones
            B2ERROR(str(e))
            continue

        objects.append(req.json())

    # we retrieved all we could, print them
    with Pager("Global tag Information"):
        for info in objects:
            print_globaltag(info)

    # return the number of tags which could not get retrieved
    return len(args.tag) - len(objects)


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

    db.request("POST", "/globalTag/{name}".format(**typeinfo), "Creating global tag {name}".format(**info),
               json=info)


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
    req = db.request("GET", "/globalTag/{}".format(args.tag),
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
    req = db.request("GET", "/globalTag/{}".format(args.tag),
                     "Getting info for global tag {}".format(args.tag))
    info = req.json()

    # now we clone the tag
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

    db.request("PUT", "/globalTag/{}/PUBLISH".format(args.tag), "Publishing global tag {}".format(args.tag))


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

    db.request("PUT", "/globalTag/{}/INVALID".format(args.tag), "invalidateing global tag {}".format(args.tag))


def command_iov(args, db):
    """
    List all IoVs defined in a global tag, optionally limited to a run range

    This command lists all IoVs defined in a given global tag. The list can be
    limited to a given run and optionally searched using --filter or
    --exclude.
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
        req = db.request("GET", "/globalTag/{tag}/globalTagPayloads".format(**vars(args)), msg)

    with Pager("List of IoVs{}{}".format(iovfilter, " (detailed)" if args.detail else "")):
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
                        ["payload modified by", payload["modifiedBy"]],
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
            table_header = ["First Exp", "First Run", "Final Exp", "Final Run", "Name", "Rev.", "IoV id", "payload id"]
            table.insert(0, table_header)
            pretty_print_table(table, [6, 6, 6, 6, '*', -8, 6, 7])


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
        print(namespace, values, option_string)
        # run in pager because amount of options will be looong
        with Pager("{} {}".format(parser.prog, option_string)):
            parser.print_help()
            self.print_subparsers(parser)
            parser.exit()


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

    # modify logging to remove the useless module: lines
    for level in LogLevel.values.values():
        logging.set_info(level, LogInfo.LEVEL | LogInfo.MESSAGE)

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--debugging", action="store_true",
                        help="Enable debugging of http traffic")
    parser.add_argument("--help-full", action=FullHelpAction,
                        help="show help message for all commands and exit")
    parser.add_argument("--base-url", default=ConditionsDB.BASE_URL,
                        help="URI for the base of the REST API (default: %(default)s)")
    parser.set_defaults(func=lambda x, y: parser.print_help())
    parsers = parser.add_subparsers(
        title="Top level commands",
        description="There are several commands to show/modify the contents of the "
        "conditions database. To get additional help, run '%(prog)s COMMAND --help'"
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
                    description="There are several commands to show/modify the contents of the "
                    "conditions database. To get additional help, run '%(prog)s COMMAND --help'"
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
        helptxt, description = func.__doc__.split("\n\n", 1)
        command_parser = parent.add_parser(parts[-1], help=helptxt, description=description)
        # now call the function with the parser as first argument and no
        # database instance. This let's them define their own arguments
        func(command_parser, None)
        # and set the function as default to be called for later
        command_parser.set_defaults(func=func)
        # also add it back to the list of subparsers
        subparsers[tuple(parts)] = [command_parser, None]

    # done, all functions parsed. Create the database instance and call the
    # correct subfunction according to the selected argument
    args = parser.parse_args()

    if args.debugging:
        enable_debugging()

    # manage some common options for up and downloading. slightly hacky but
    # need to be given to ConditionsDB on construction so meh
    nprocess = getattr(args, "nprocess", 1)
    retries = getattr(args, "retries", 1)
    # need at least one worker thread
    if nprocess <= 0:
        B2WARNING("-j must be larger than zero, ignoring")
        args.nprocess = nprocess = 1

    conditions_db = ConditionsDB(args.base_url, nprocess, retries)
    try:
        return args.func(args, conditions_db)
    except ConditionsDB.RequestError as e:
        B2ERROR(str(e))
        return 1

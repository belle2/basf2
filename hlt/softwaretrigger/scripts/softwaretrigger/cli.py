#!/usr/bin/env python3
import argparse
import os
import tempfile
import shutil
import json
from concurrent.futures import ThreadPoolExecutor
import difflib
import pandas as pd

import basf2
from conditions_db import cli_download, ConditionsDB, encode_name
from softwaretrigger import db_access


class HashableCut(dict):
    """Small helper class as the difflib does not understand dicts directly (as they are not hashable)"""

    def __hash__(self):
        """Create a hash for the object out of the json string"""
        return hash(json.dumps(self))


class DownloadableDatabase:
    """Helper class to translate the user-specified database(s) into parameters for basf2"""

    def __init__(self, command_argument):
        """Init the stored databases and exp/run from the specified command argument"""
        #: the specified databases
        self._database = []
        #: the experiment number, default (= latest) is 99999
        self._experiment = 99999
        #: the run number, default (= latest) is 99999
        self._run = 99999

        # If given, use the experiment run from the command_argument
        split_argument = command_argument.split(":")
        if len(split_argument) == 2:
            command_argument, exp_run = split_argument

            if exp_run != "latest":
                try:
                    self._experiment, self._run = map(int, exp_run.split("/"))
                except BaseException:
                    raise argparse.ArgumentTypeError(
                        f"Do not understand the exp/run argument '{exp_run}'")

        elif len(split_argument) != 1:
            raise argparse.ArgumentTypeError(
                f"Do not understand the database argument '{command_argument}'")

        # Now split up the databases
        self._database = command_argument.split(",")

        # However make sure we have them in the correct format
        def normalize(database):
            # In case a local file is specified we can just use it directly
            if os.path.exists(database):
                if os.path.basename(database) != "database.txt":
                    database = os.path.join(database, "database.txt")

            return database

        self._database = list(map(normalize, self._database))

    def set_database(self):
        """
        Set the basf2 database chain according to the specified databases.
        Before that, clean up and invalidate everything from th database.

        The distinction between file databases and global databases is done
        via the fact of a file/folder with this name exists or not.
        """
        from ROOT import Belle2
        Belle2.DBStore.Instance().reset()

        basf2.reset_database()
        basf2.use_database_chain()

        for database in self._database:
            if os.path.exists(database):
                basf2.use_local_database(database)
            else:
                basf2.use_central_database(database)

        db_access.set_event_number(evt_number=0, run_number=int(self._run),
                                   exp_number=int(self._experiment))

    def get_all_cuts(self):
        """
        Get all cuts stored in the database(s)
        and sort them according to base_identifier, cut_identifier.
        """
        self.set_database()

        all_cuts = db_access.get_all_cuts()
        all_cuts = sorted(all_cuts,
                          key=lambda cut: (cut["Base Identifier"], cut["Cut Identifier"]))
        all_cuts = list(map(HashableCut, all_cuts))
        return all_cuts


def diff_function(args):
    """
    Show the diff between two specified databases.
    """
    first_database_cuts = args.first_database.get_all_cuts()
    second_database_cuts = args.second_database.get_all_cuts()

    diff = difflib.SequenceMatcher(
        a=list(map(str, first_database_cuts)), b=list(map(str, second_database_cuts)))

    def print_cut(cut, prefix=" "):
        if prefix == "-":
            print("\x1b[31m", end="")
        elif prefix == "+":
            print("\x1b[32m", end="")
        print(prefix, cut)
        print("\x1b[0m", end="")

    for c, i1, i2, j1, j2 in diff.get_opcodes():
        if c == "equal":
            if args.only_changes:
                continue
            assert len(range(i1, i2)) == len(range(j1, j2))
            for i in range(i1, i2):
                print_cut(diff.a[i])
        elif c == "replace":
            assert len(range(i1, i2)) == len(range(j1, j2))
            for i, j in zip(range(i1, i2), range(j1, j2)):
                print_cut(diff.a[i], "-")
                print_cut(diff.b[j], "+")
        elif c in "delete":
            for i in range(i1, i2):
                print_cut(diff.a[i], prefix="-")
        elif c == "insert":
            for j in range(j1, j2):
                print_cut(diff.b[j], "+")
        else:
            raise ValueError("Do not understand the output of diff!")


def add_cut_function(args):
    """
    Add a cut with the given parameters and also add it to the trigger menu.
    """
    args.database.set_database()

    db_access.upload_cut_to_db(cut_string=args.cut_string, base_identifier=args.base_identifier,
                               cut_identifier=args.cut_identifier, prescale_factor=args.prescale_factor,
                               reject_cut=args.reject_cut.lower() == "true", iov=None)
    trigger_menu = db_access.download_trigger_menu_from_db(args.base_identifier,
                                                           do_set_event_number=False)
    cuts = list(trigger_menu.getCutIdentifiers())

    if args.cut_identifier not in cuts:
        cuts.append(args.cut_identifier)

        db_access.upload_trigger_menu_to_db(args.base_identifier, cuts,
                                            accept_mode=trigger_menu.isAcceptMode(), iov=None)


def remove_cut_function(args):
    """
    Remove a cut with the given name from the trigger menu.
    """
    args.database.set_database()

    trigger_menu = db_access.download_trigger_menu_from_db(
        args.base_identifier, do_set_event_number=False)
    cuts = trigger_menu.getCutIdentifiers()

    cuts = [cut for cut in cuts if cut != args.cut_identifier]
    db_access.upload_trigger_menu_to_db(
        args.base_identifier, cuts, accept_mode=trigger_menu.isAcceptMode(), iov=None)


def print_function(args):
    """
    Print the cuts stored in the database(s).
    """
    cuts = args.database.get_all_cuts()
    df = pd.DataFrame(cuts)

    if args.format == "pandas":
        pd.set_option("display.max_rows", 500)
        pd.set_option("display.max_colwidth", 200)
        pd.set_option('display.max_columns', 500)
        pd.set_option('display.width', 1000)
        print(df)
    elif args.format == "jira":
        from tabulate import tabulate
        print(tabulate(df, tablefmt="jira", showindex=False, headers="keys"))
    elif args.format == "grid":
        from tabulate import tabulate
        print(tabulate(df, tablefmt="grid", showindex=False, headers="keys"))
    elif args.format == "json":
        import json
        print(json.dumps(df.to_dict("records"), indent=2))
    elif args.format == "list":
        for base_identifier, cuts in df.groupby("Base Identifier"):
            for _, cut in cuts.iterrows():
                print(cut["Base Identifier"], cut["Cut Identifier"])
    elif args.format == "human-readable":
        print("Currently, the following menus and triggers are in the database")
        for base_identifier, cuts in df.groupby("Base Identifier"):
            print(base_identifier)
            print("")
            print("\tUsed triggers:\n\t\t" +
                  ", ".join(list(cuts["Cut Identifier"])))
            print("\tIs in accept mode:\n\t\t" +
                  str(cuts["Reject Menu"].iloc[0]))
            for _, cut in cuts.iterrows():
                print("\t\tCut Name:\n\t\t\t" + cut["Cut Identifier"])
                print("\t\tCut condition:\n\t\t\t" + cut["Cut Condition"])
                print("\t\tCut prescaling\n\t\t\t" +
                      str(cut["Cut Prescaling"]))
                print("\t\tCut is a reject cut:\n\t\t\t" +
                      str(cut["Reject Cut"]))
                print()
    else:
        raise AttributeError(f"Do not understand format {args.format}")


def main():
    """
    Main function to be called from b2hlt_triggers.
    """
    parser = argparse.ArgumentParser(
        description="""
Execute different actions on stored trigger menus in the database.

Call with `%(prog)s [command] --help` to get a description on each command.
Please also see the examples at the end of this help.

Many commands require one (or many) specified databases. Different formats are possible.
All arguments need to be written in quotation marks.
* "online"                      Use the latest version in the "online" database
                                (or any other specified global tag).
* "online:latest"               Same as just "online", makes things a bit clearer.
* "online:8/345"                Use the version in the "online" database (or any other specified global tag)
                                which was present in exp 8 run 345.
* "localdb:4/42"                Use the local database specified in the given folder for the given exp/run.
* "localdb/database.txt"        It is also possible to specify a file directly.
* "online,localdb"              First look in localdb, then in the online GT
* "online,localdb:9/1"          Can also be combined with the exp/run (It is then valid for all database accesses)

Examples:

* Check what has changed between 8/1 and 9/1 in the online GT.

    %(prog)s diff --first-database "online:8/1" --second-database "online:9/1" --only-changes

* Especially useful while editing trigger cuts and menus: check what has changed between the latest
  version online and what is currently additionally in localdb

    %(prog)s diff --first-database "online:latest" --second-database "online,localdb:latest"

  This use case is so common, it is even the default

    %(prog)s diff

* Print the latest version of the cuts in online (plus what is defined in the localdb) in a human-friendly way

    %(prog)s print

* Print the version of the cuts which was present in 8/1 online in a format understandable by JIRA
  (you need to have the tabulate package installed)

    %(prog)s print --database "online:8/1" --format jira

* Add a new skim cut named "accept_b2bcluster_3D" with the specified parameters and upload it to localdb

    %(prog)s add_cut skim accept_b2bcluster_3D "[[nB2BCC3DLE >= 1] and [G1CMSBhabhaLE < 2]]" 1 False

* Remove the cut "accept_bhabha" from the trigger menu "skim"

    %(prog)s remove_cut skim accept_bhabha

        """,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        usage="%(prog)s command"
    )
    parser.set_defaults(func=lambda *args: parser.print_help())
    subparsers = parser.add_subparsers(title="command",
                                       description="Choose the command to execute")

    # diff command
    diff_parser = subparsers.add_parser("diff", help="Compare the trigger menu in two different databases.",
                                        formatter_class=argparse.RawDescriptionHelpFormatter,
                                        description="""
Compare the two trigger menus present in the two specified databases
(or database chains) for the given exp/run combination (or the latest
version).
Every line in the output is one trigger line. A "+" in front means the
trigger line is present in the second database, but not in the first.
A "-" means exactly the opposite. Updates trigger lines will show up
as both "-" and "+" (with different parameters).

The two databases (or database chains) can be specified as describes
in the general help (check b2hlt_triggers --help).
By default, the latest version of the online database will be
compared with what is defined on top in the localdb.
                                         """)
    diff_parser.add_argument("--first-database", help="First database to compare. Defaults to 'online:latest'.",
                             type=DownloadableDatabase, default=DownloadableDatabase("online:latest"))
    diff_parser.add_argument("--second-database", help="Second database to compare. Defaults to 'online,localdb:latest'.",
                             type=DownloadableDatabase, default=DownloadableDatabase("online,localdb:latest"))
    diff_parser.add_argument(
        "--only-changes", help="Do not show unchanged lines.", action="store_true")
    diff_parser.set_defaults(func=diff_function)

    # print command
    print_parser = subparsers.add_parser("print", help="Print the cuts stored in the given database.",
                                         formatter_class=argparse.RawDescriptionHelpFormatter,
                                         description="""
Print the defined trigger menu and trigger cuts in a human-friendly
(default) or machine-friendly way.
The database (or database chain) needs to be specified in the general
help (check b2hlt_triggers --help).

For additional formatting options please install the tabulate package with

    pip3 install --user tabulate

By default the latest version on the online database and what is defined on
top in the localdb will be shown.
                                         """)
    print_parser.add_argument("--database", help="Which database to print. Defaults to 'online,localdb:latest'.",
                              type=DownloadableDatabase, default=DownloadableDatabase("online,localdb:latest"))
    choices = ["human-readable", "json", "list", "pandas"]
    try:
        from tabulate import tabulate
        choices += ['jira', 'grid']
    except ImportError:
        pass
    print_parser.add_argument("--format", help="Choose the format how to print the trigger cuts. "
                              "To get access to more options please install the tabulate package using pip",
                              choices=choices, default="human-readable")
    print_parser.set_defaults(func=print_function)

    # add_cut command
    add_cut_parser = subparsers.add_parser("add_cut", help="Add a new cut.",
                                           formatter_class=argparse.RawDescriptionHelpFormatter,
                                           description="""
Add a cut with the given properties and upload it into the localdb database.
After that, you can upload it to the central database, to e.g. staging_online.

As a base line for editing, a database much be specified in the usual format
(check b2hlt_triggers --help).
It defaults to the latest version online and the already present changes in
localdb.
Please note that the IoV of the created trigger line and menu is set to infinite.
                                           """)
    add_cut_parser.add_argument("--database", help="Where to take the trigger menu from. Defaults to 'online,localdb:latest'.",
                                type=DownloadableDatabase, default=DownloadableDatabase("online,localdb:latest"))
    add_cut_parser.add_argument("base_identifier",
                                help="base_identifier of the cut to add", choices=["filter", "skim"])
    add_cut_parser.add_argument("cut_identifier",
                                help="cut_identifier of the cut to add")
    add_cut_parser.add_argument("cut_string",
                                help="cut_string of the cut to add")
    add_cut_parser.add_argument("prescale_factor", type=int,
                                help="prescale of the cut to add")
    add_cut_parser.add_argument(
        "reject_cut", help="Is the new cut a reject cut?")
    add_cut_parser.set_defaults(func=add_cut_function)

    # remove_cut command
    remove_cut_parser = subparsers.add_parser("remove_cut", help="Remove a cut of the given name.",
                                              formatter_class=argparse.RawDescriptionHelpFormatter,
                                              description="""
Remove a cut with the given base and cut identifier from the trigger menu
and upload the new trigger menu to the localdb.
After that, you can upload it to the central database, to e.g. staging_online.

As a base line for editing, a database much be specified in the usual format
(check b2hlt_triggers --help).
It defaults to the latest version online and the already present changes in
localdb.
Please note that the IoV of the created trigger menu is set to infinite.

The old cut payload will not be deleted from the database. This is not
needed as only cuts specified in a trigger menu are used.
                                              """)
    remove_cut_parser.add_argument("base_identifier",
                                   help="base_identifier of the cut to delete", choices=["filter", "skim"])
    remove_cut_parser.add_argument("cut_identifier",
                                   help="cut_identifier of the cut to delete")
    remove_cut_parser.add_argument("--database",
                                   help="Where to take the trigger menu from. Defaults to 'online,localdb:latest'.",
                                   type=DownloadableDatabase, default=DownloadableDatabase("online,localdb:latest"))
    remove_cut_parser.set_defaults(func=remove_cut_function)

    args = parser.parse_args()
    args.func(args)

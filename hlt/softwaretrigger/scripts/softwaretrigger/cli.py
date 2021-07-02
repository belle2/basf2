#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import argparse
import os
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

        basf2.conditions.override_globaltags()

        for database in self._database:
            if os.path.exists(database):
                basf2.conditions.prepend_testing_payloads(database)
            else:
                basf2.conditions.prepend_globaltag(database)

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

    def print_cuts(prefix, cuts):
        for c in cuts:
            print_cut(c, prefix)

    for tag, i1, i2, j1, j2 in diff.get_opcodes():
        if tag == "equal":
            if args.only_changes:
                continue
            print_cuts(" ", diff.b[j1:j2])
        if tag in ["delete", "replace"]:
            print_cuts("-", diff.a[i1:i2])
        if tag in ["insert", "replace"]:
            print_cuts("+", diff.b[j1:j2])


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


def create_script_function(args):
    """
    Print the b2hlt_trigger commands to create a lobal database copy.
    """
    cuts = args.database.get_all_cuts()
    df = pd.DataFrame(cuts)

    sfmt = 'b2hlt_triggers add_cut \
"{Base Identifier}" "{Cut Identifier}" "{Cut Condition}" "{Cut Prescaling}" "{Reject Cut}"'.format
    if args.filename is None:
        df.apply(lambda x: print(sfmt(**x)), 1)
    else:
        with open(args.filename, 'w') as f:
            df.apply(lambda x: f.write(sfmt(**x) + '\n'), 1)


def iov_includes(iov_list, exp, run):
    """
    Comparison function between two IoVs (start, end) stored in the database and
    the given exp/run combination.
    """
    # Dirty hack: replace -1 by infinity to make the comparison easier
    copied_iov_list = iov_list[2:]
    copied_iov_list = list(map(lambda x: x if x != -1 else float("inf"), copied_iov_list))

    exp_start, run_start, exp_end, run_end = copied_iov_list

    return (exp_start, run_start) <= (exp, run) <= (exp_end, run_end)


def download_function(args):
    """
    Download the trigger cuts in the given database to disk and set their IoV to infinity.
    """
    if len(args.database._database) != 1:
        raise AttributeError("Can only download from a single database! Please do not specify more than one.")

    global_tag = args.database._database[0]

    # The following is an adapted version of cli_download
    os.makedirs(args.destination, exist_ok=True)

    db = ConditionsDB()
    req = db.request("GET", f"/globalTag/{encode_name(global_tag)}/globalTagPayloads",
                     f"Downloading list of payloads for {global_tag} tag")

    download_list = {}
    for payload in req.json():
        name = payload["payloadId"]["basf2Module"]["name"]
        if not name.startswith("software_trigger_cut"):
            continue

        local_file, remote_file, checksum, iovlist = cli_download.check_payload(args.destination, payload)

        new_iovlist = list(filter(lambda iov: iov_includes(iov, args.database._experiment, args.database._run), iovlist))
        if not new_iovlist:
            continue

        if local_file in download_list:
            download_list[local_file][-1] += iovlist
        else:
            download_list[local_file] = [local_file, remote_file, checksum, iovlist]

    # do the downloading
    full_iovlist = []
    failed = 0
    with ThreadPoolExecutor(max_workers=20) as pool:
        for iovlist in pool.map(lambda x: cli_download.download_file(db, *x), download_list.values()):
            if iovlist is None:
                failed += 1
                continue

            full_iovlist += iovlist

    dbfile = []
    for iov in sorted(full_iovlist):
        # Set the IoV intentionally to 0, 0, -1, -1
        iov = [iov[0], iov[1], 0, 0, -1, -1]
        dbfile.append("dbstore/{} {} {},{},{},{}\n".format(*iov))
    with open(os.path.join(args.destination, "database.txt"), "w") as txtfile:
        txtfile.writelines(dbfile)


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

* Download the latest state of the triggers into the folder "localdb", e.g. to be used for local studies

    %(prog)s download

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
        choices += ['jira', 'grid']
    except ImportError:
        pass
    print_parser.add_argument("--format", help="Choose the format how to print the trigger cuts. "
                              "To get access to more options please install the tabulate package using pip",
                              choices=choices, default="human-readable")
    print_parser.set_defaults(func=print_function)

    # create-script command
    create_script_parser = subparsers.add_parser(
        "create_script",
        help="Create b2hlt_triggers command to create a online globaltag copy.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="""
Generate the required b2hlt_trigger commands to reproduce an online globaltag for a given exp/run
number to create a local database version of it.
                                                 """)
    create_script_parser.add_argument("--database", help="Which database to print. Defaults to 'online:latest'.",
                                      type=DownloadableDatabase, default=DownloadableDatabase("online:latest"))
    create_script_parser.add_argument("--filename", default=None,
                                      help="Write to given filename instead of stdout.")
    create_script_parser.set_defaults(func=create_script_function)

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

    # download command
    download_parser = subparsers.add_parser("download", help="Download the trigger menu from the database.",
                                            formatter_class=argparse.RawDescriptionHelpFormatter,
                                            description="""
Download all software trigger related payloads from the specified database
into the folder localdb and create a localdb/database.txt. This is
especially useful when doing local trigger studies which should use the
latest version of the online triggers. By default, the latest
version of the online GT will be downloaded.

Attention: this script will override a database defined in the destination
folder (default localdb)!
Attention 2: all IoVs of the downloaded triggers will be set to 0, 0, -1, -1
so you can use the payloads fro your local studies for whatever run you want.
This should not (never!) be used to upload or edit new triggers and
is purely a convenience function to synchronize your local studies
with the online database!

Please note that for this command you can only specify a single database
(all others can work with multiple databases).
                                              """)
    download_parser.add_argument("--database",
                                 help="Single database where to take the trigger menu from. Defaults to 'online:latest'.",
                                 type=DownloadableDatabase, default=DownloadableDatabase("online:latest"))
    download_parser.add_argument("--destination",
                                 help="In which folder to store the output", default="localdb")
    download_parser.set_defaults(func=download_function)

    args = parser.parse_args()
    args.func(args)

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import functools
import time
from fnmatch import fnmatch
from concurrent.futures import ThreadPoolExecutor
from collections import defaultdict
from basf2 import B2INFO, B2ERROR, B2WARNING, LogPythonInterface  # noqa
from basf2.utils import pretty_print_table
from terminal_utils import Pager
from conditions_db import set_cdb_authentication_token
from conditions_db.iov import IoVSet, IntervalOfValidity
from conditions_db.runningupdate import RunningTagUpdater, RunningTagUpdaterError, RunningTagUpdateMode
from conditions_db.cli_utils import ItemFilter


def get_all_iovsets(existing_payloads, run_range=None):
    """Given a list of PayloadInformation objects, return a reduced list PayloadInformation
    objects with the single iovs replaced with IoVSets. Payloads with the same
    name and revision will be merged.

    Overlaps will raise an B2ERROR
    """
    all_payloads = defaultdict(lambda: IoVSet(allow_overlaps=True))
    by_name = defaultdict(lambda: IoVSet(allow_overlaps=False))
    infos = {}
    for payload in existing_payloads:
        # we want to make set of iovs for each payload
        iov = IntervalOfValidity(payload.iov)
        # possibly we have a run range we want to limit to
        if run_range is not None:
            iov &= run_range
            if not iov:
                continue

        # merge the iovs for the same revision
        all_payloads[payload.name, payload.revision].add(iov)
        # also keep the PayloadInformation
        infos[payload.name, payload.revision] = payload

        # and also check if there are any overlaps with the same payload name
        try:
            by_name[payload.name].add(iov)
        except ValueError as e:
            B2ERROR(f"Overlap for payload {payload.name} r{payload.revision}: {e}")

    # so now flatten the thing again and return PayloadInformation objects we slightly modify
    result = []
    for (name, revision), iov in all_payloads.items():
        info = infos[name, revision]
        info.iov = iov
        info.iov_id = None
        result.append(info)

    return result


def create_iov_wrapper(db, globaltag_id, payload):
    """
    Wrapper function for adding payloads into a given globaltag.
    """
    for iov in payload.iov:
        if db.create_iov(globaltag_id, payload.payload_id, *iov.tuple) is None:
            raise RuntimeError(f"Cannot create iov for {payload.name} r{payload.revision}")


def command_tag_merge(args, db=None):
    """
    Merge a list of globaltags in the order they are given.

    This command allows to merge a number of globaltags into a single globaltag.
    Payloads from later globaltags in the list of arguments are used to fill gaps
    present in earlier globaltags.

    The result is equivalent to having multiple globaltags setup in the conditions
    access for basf2 (highest priority goes first).

    .. warning::

      The order of the globaltags is highest priority first, so payloads from
      globaltags earlier on the command line will be taken with before globaltags
      from later tags.

    For each globaltag in the list we copy all payloads to the output globaltag
    if there is no payload of that name valid for the given interval of validity
    in any previous globaltags in the list.

    If the payload overlaps partially with a payload from a previous globaltag
    in the list the interval of validity is shortened (and possibly split) to
    not overlap but to just fill the gaps.

    For example:

    Globaltag ``A`` contains ::

        payload1, rev 2, valid from 1,0 to 1,10
        payload1, rev 3, valid from 1,20 to 1,22
        payload2, rev 1, valid from 1,0 to 1,-1

    Globaltag ``B`` contains ::

        payload1, rev 1, valid from 1,1 to 1,30
        payload2, rev 2, valid from 0,1 to 1,20

    Then running ``b2conditionsdb tag merge -o C A B``, the output globaltag ``C``
    after the merge will contain::

        payload1, rev 2, valid from 1,0 to 1,10
        payload1, rev 1, valid from 1,11 to 1,19
        payload1, rev 3, valid from 1,20 to 1,22
        payload1, rev 1, valid from 1,23 to 1,30
        payload2, rev 2, valid from 0,1 to 0,-1
        payload2, rev 1, valid from 1,0 to 1,-1

    When finished, this command will print a table of payloads and their
    validity and from which globaltag they were taken. If ``--dry-run`` is given
    it will only print the list of payloads.

    Optionally one can specify ``--run-range`` to limit the run range for which
    the merging should take place. In the example above, running with
    ``--run-range 1 0 1 21`` the result would be ::

        payload1, rev 2, valid from 1,0 to 1,10
        payload1, rev 1, valid from 1,11 to 1,19
        payload1, rev 3, valid from 1,20 to 1,21
        payload2, rev 1, valid from 1,0 to 1,21

    .. versionadded:: release-05-01-00
    """

    if db is None:
        args.add_argument("globaltag", nargs="+", help="name of the globaltag")
        group = args.add_argument_group("required named arguments")
        group.add_argument("-o", "--output", required=True, help="Name of the output globaltag")
        args.add_argument("--dry-run", help="Don't do anything, just print a table with the results",
                          action="store_true", default=False)
        args.add_argument("--run-range", nargs=4, default=None, type=int,
                          metavar=("FIRST_EXP", "FIRST_RUN", "FINAL_EXP", "FINAL_RUN"),
                          help="Can be for numbers to limit the run range to put"
                          "in the output globaltag: All iovs will be limited to "
                          "be in this range.")
        args.add_argument("-j", type=int, default=10, dest="nprocess",
                          help="Number of concurrent threads to use for "
                          "creating payloads into the output globaltag.")
        return

    if not args.dry_run:
        set_cdb_authentication_token(db, args.auth_token)

    # prepare some colors for easy distinction of source tag
    support_color = LogPythonInterface.terminal_supports_colors()
    if support_color:
        colors = "\x1b[32m \x1b[34m \x1b[35m \x1b[31m".split()
        colors = {tag: color for tag, color in zip(args.globaltag, colors)}

    def color_row(row, _, line):
        """Color the lines depending on which globaltag the payload comes from"""
        if not support_color:
            return line
        begin = colors.get(row[-1], "")
        end = '\x1b[0m'
        return begin + line + end

    with Pager("Result of merging globaltags", True):
        # make sure output tag exists
        output_id = db.get_globalTagInfo(args.output)
        if output_id is None:
            B2ERROR("Output globaltag doesn't exist. Please create it first with a proper description")
            return False

        output_id = output_id["globalTagId"]

        # check all globaltags exist
        if any(db.get_globalTagInfo(tag) is None for tag in args.globaltag):
            return False

        final = []
        table = []
        existing = defaultdict(lambda: IoVSet(allow_overlaps=True))
        if args.run_range is not None:
            args.run_range = IntervalOfValidity(args.run_range)

        # For each globaltag
        for tag in args.globaltag:
            # get all the payloads and iovs from the globaltag
            all_payloads = db.get_all_iovs(tag)
            # sort all the payloads by revision number (reversed sort: highest revisions first)
            all_payloads.sort(key=lambda p: p.revision, reverse=True)
            # and sort again but this time by name: not really necessary,
            # but it helps printing the log messages ordered by payloads name
            all_payloads.sort(key=lambda p: p.name, reverse=False)
            # get all payload information objects with their iovs already merged to iovset instances
            payloads = get_all_iovsets(all_payloads, args.run_range)
            for payload in payloads:
                # make sure it doesn't overlap with any of the previous
                payload.iov.remove(existing[payload.name])
                # but if there's something left
                if payload.iov:
                    # extend the known coverage of this payload
                    existing[payload.name] |= payload.iov
                    # extend this payload to the list to create later
                    final.append(payload)
                    # and add all separate iovs to the table to show the user
                    for iov in payload.iov:
                        table.append([payload.name, payload.revision] + list(iov.tuple) + [tag])

        # sort the table by payload name and start run ... we want to display it
        table.sort(key=lambda r: (r[0], r[3:5]))

        # and fancy print it ...
        table.insert(0, ["Name", "Rev", "First Exp", "First Run", "Final Exp", "Final Run", "Source"])
        columns = ["+", -8, 6, 6, 6, 6, max(len(_) for _ in args.globaltag)]

        B2INFO(f"Result of merging the globaltags {', '.join(args.globaltag)}")

        pretty_print_table(table, columns, transform=color_row)

    # Ok, we're still alive, create all the payloads using multiple processes.
    if not args.dry_run:
        B2INFO(f'Now copying the {len(final)} payloads into {args.output} to create {len(table)-1} iovs ...')
        create_iov = functools.partial(create_iov_wrapper, db, output_id)
        try:
            with ThreadPoolExecutor(max_workers=args.nprocess) as pool:
                start = time.monotonic()
                for payload, _ in enumerate(pool.map(create_iov, final), 1):
                    eta = (time.monotonic() - start) / payload * (len(final) - payload)
                    B2INFO(f"{payload}/{len(final)} payloads copied, ETA: {eta:.1f} seconds")
        except RuntimeError:
            B2ERROR("Not all iovs could be created. This could be a server/network problem "
                    "or the destination globaltag was not empty or not writeable. Please make "
                    "sure the target tag is empty and try again")
            return 1

    return 0


def command_tag_runningupdate(args, db=None):
    """
    Update a running globaltag with payloads from a staging tag

    This command will calculate and apply the necessary updates to a running
    globaltag with a given staging globaltag

    Running tags are defined as "immutable for existing data but conditions for
    newer runs may be added" and the only modification allowed is to add new
    payloads for new runs or close existing payloads to no longer be valid for
    new runs.

    This command takes previously prepared and validated payloads in a staging
    globaltag and will then calculate which payloads to close and what to add to
    the running globaltag.

    For this to work we require

    1. A running globaltag in the state "RUNNING"

    2. A (experiment, run) number from which run on the update should be valid.
       This run number needs to be

        a) bigger than the start of validity for all iovs in the running tag
        b) bigger than the end of validity for all closed iovs (i.e. not valid
           to infinity) in the running tag

    3. A staging globaltag with the new payloads in state "VALIDATED"

        a) payloads in the staging tag starting at (0,0) will be interpreted as
           starting at the first valid run for the update
        b) all other payloads need to start at or after the first valid run for
           the update.
        c) The globaltag needs to be gap and overlap free
        d) All payloads in the staging tag should have as last iov an open iov
           (i.e. valid to infinity) but this can be disabled.

    The script will check all the above requirements and will then calculate the
    necessary operations to

    1. Add all payloads from the staging tag where a start validity of (0, 0) is
       replaced by the starting run for which this update should be valid.
    2. close all iovs for payloads in the running tags just before the
       corresponding iov of the same payload in the staging tag, so either at the
       first run for the update to be valid or later
    3. Optionally, make sure all payloads in the staging tag end in an open iov.

    .. rubric:: Examples

    Running tag contains ::

        payload1, rev 1, valid from 0,1 to 1,0
        payload1, rev 2, valid from 1,1 to -1,-1
        payload2, rev 1, valid from 0,1 to -1,-1
        payload3, rev 1, valid from 0,1 to 1,0
        payload4, rev 1, valid from 0,1 to -1,-1
        payload5, rev 1, valid from 0,1 to -1,-1

    Staging tag contains ::

        payload1, rev 3, valid from 0,0 to 1,8
        payload1, rev 4, valid from 1,9 to 1,20
        payload2, rev 2, valid from 1,5 to 1,20
        payload3, rev 2, valid from 0,0 to -1,-1
        payload4, rev 1, valid from 0,0 to 1,20

    Then running ``b2conditionsdb tag runningupdate running staging --run 1 2 --allow-closed``,
    the running globaltag after the update will contain ::

        payload1, rev 1, valid from 0,1 to 1,0
        payload1, rev 2, valid from 1,1 to 1,1
        payload1, rev 3, valid from 1,2 to 1,8
        payload1, rev 4, valid from 1,9 to 1,20
        payload2, rev 1, valid from 0,1 to 1,4
        payload2, rev 2, valid from 1,5 to 1,20
        payload3, rev 1, valid from 0,1 to 1,0
        payload3, rev 2, valid from 1,2 to -1,-1
        payload4, rev 1, valid from 0,1 to 1,20
        payload5, rev 1, valid from 0,1 to -1,-1

    Note that

        - the start of payload1 and payload3 in staging has been adjusted

        - payload2 in the running tag as been closed at 1,4, just before the
          validity from the staging tag

        - payload3 was already closed in the running tag so no change is
          performed. This might result in gaps but is intentional

        - payload4 was not closed at rim 1,2 because the staging tag had the same
          revision of the payload so the these were merged to one long validity.

        - payload5 was not closed as there was no update to it in the staging tag.
          If we would have run with ``--full-replacement`` it would have been closed.

        - if we would have chosen ``--run 1 1`` the update would have failed because
          payload1, rev2 in running starts at 1,1 so we would have a conflict

        - if we would have chosen ``--run 1 6`` the update would have failed because
          payload2 in the staging tag starts before this run

        - if we would have chosen to open the final iovs in staging by using
          ``--fix-closed``, payload1, rev 4; payload2, rev 2 and payload4 rev 1
          would be valid until -1,-1 after the running tag. In fact, payload 4
          would not be changed at all.
    """
    if db is None:
        args.add_argument("running", help="name of the running globaltag")
        args.add_argument("staging", help="name of the staging globaltag")
        group = args.add_argument_group("required named arguments")
        group.add_argument("-r", "--run", required=True, nargs=2, type=int, metavar=("EXP", "RUN"),
                           help="First experiment + run number for which the update should be "
                           "valid. Two numbers separated by space")
        choice = args.add_mutually_exclusive_group()
        choice.add_argument("--allow-closed", dest="mode", action="store_const",
                            const=RunningTagUpdateMode.ALLOW_CLOSED,
                            default=RunningTagUpdateMode.STRICT,
                            help="if given allow payloads in the staging tag to not "
                            "be open, i.e. they don't have to be open ended in the "
                            "update. Useful to retire a payload by adding one last update")
        choice.add_argument("--fix-closed", dest="mode", action="store_const",
                            const=RunningTagUpdateMode.FIX_CLOSED,
                            help="if given automatically open the last iov for each "
                            "payload in staging if it is closed.")
        choice.add_argument("--simple-mode", dest="mode", action="store_const",
                            const=RunningTagUpdateMode.SIMPLE,
                            help="if given require the staging tag to solely consist "
                            "of fully infinite validities: Only one iov per payload "
                            "with a validity of (0,0,-1,-1)")
        choice.add_argument("--full-replacement", dest="mode", action="store_const",
                            const=RunningTagUpdateMode.FULL_REPLACEMENT,
                            help="if given perform a full replacement and close all "
                            "open iovs in the running tag not present in the staging tag. "
                            "After such an update exactly the payloads in the staging tag "
                            "will be valid after the given run. This allows for closed iovs "
                            "in the staging tag as with ``--allow-closed``")
        args.add_argument("--dry-run", default=False, action="store_true",
                          help="Only show the changes, don't try to apply them")
        return

    if not args.dry_run:
        set_cdb_authentication_token(db, args.auth_token)

    try:
        updater = RunningTagUpdater(db, args.running, args.staging, args.run, args.mode, args.dry_run)
        operations = updater.calculate_update()
    except RunningTagUpdaterError as e:
        B2ERROR(e, **e.extra_vars)
        return 1

    # make sure we exit if we have nothing to do
    if not operations:
        B2INFO("Nothing to do, please check the globaltags given are correct")
        return 1

    # show operations in a table and some summary
    table = []
    last_valid = tuple(args.run)
    summary = {
        "first valid run": last_valid,
        "payloads closed": 0,
        "payloads updated": 0,
        "payload iovs added": 0,
        "next possible update": last_valid,
    }

    updated_payloads = set()
    for op, payload in operations:
        # calculate how many payloads/iovs will be closed/added
        if op == "CLOSE":
            summary['payloads closed'] += 1
        else:
            updated_payloads.add(payload.name)
            summary['payload iovs added'] += 1
        # remember the highest run number of any iov, so first run
        last_valid = max(payload.iov[:2], last_valid)
        # and final run if not open
        if payload.iov[2:] != (-1, -1):
            last_valid = max(payload.iov[2:], last_valid)
        # and add to the table of operations to be shown
        table.append([op, payload.name, payload.revision] + list(payload.iov))

    # calculate the next fee run
    summary['next possible update'] = (last_valid[0] + (1 if last_valid[1] < 0 else 0), last_valid[1] + 1)
    # and the number of distinct payloads
    summary['payloads updated'] = len(updated_payloads)

    # prepare some colors for easy distinction of closed payloads
    support_color = LogPythonInterface.terminal_supports_colors()

    def color_row(row, _, line):
        """Color the lines depending on which globaltag the payload comes from"""
        if not support_color:
            return line
        begin = "" if row[0] != "CLOSE" else "\x1b[31m"
        end = '\x1b[0m'
        return begin + line + end

    # and then show the table
    table.sort(key=lambda x: (x[1], x[3:], x[2], x[0]))
    table.insert(0, ["Action", "Payload", "Rev", "First Exp", "First Run", "Final Exp", "Final Run"])
    columns = [6, '*', -6, 6, 6, 6, 6]

    with Pager(f"Changes to running tag {args.running}:", True):
        B2INFO(f"Changes to be applied to the running tag {args.running}")
        pretty_print_table(table, columns, transform=color_row)

    if args.dry_run:
        B2INFO("Running in dry mode, not applying any changes.", **summary)
        return 0

    B2WARNING("Applying these changes cannot be undone and further updates to "
              "this run range will **NOT** be possible", **summary)
    # ask if the user really knows what they're doing
    answer = input("Are you sure you want to continue? [yes/No]: ")
    while answer.lower().strip() not in ['yes', 'no', 'n', '']:
        answer = input("Please enter 'yes' or 'no': ")

    if answer.lower().strip() != 'yes':
        B2INFO("Aborted by user ...")
        return 1

    # Ok, all set ... apply the update
    try:
        updater.apply_update()
        B2INFO("done")
        return 0
    except RunningTagUpdaterError as e:
        B2ERROR(e, **e.extra_vars)
        return 1


def command_iovs(args, db=None):
    """
    Modify, delete or copy iovs from a globaltags.

    This command allows to modify, delete or copy iovs of a globaltag. If no other command is given do nothing.
    """

    # no arguments to define, just a group command
    pass

# Helper class to unify common parts of commands_iovs_*


class CommandIoVsHelper:
    """
    Class to unify common parts of b2conditionsdb iovs commands

    This class defines common argparse arguments,
    common filter of iovs and common multithreading
    """

    def __init__(self, whichcommand, args, db):
        """initialization, just remember the arguments or parser and the database instance

        Args:
            whichcommand (str): from whichcommand it is called (copy, delete or modify)
            args (argparse.ArgumentParser): where to append new arguments
            db (conditions_db.ConditionsDB): database instance to be used
        """

        #: from whichcommand it is called (copy, delete or modify)
        self.whichcommand = whichcommand
        #: argparse.ArgumentParser instance
        self._args = args
        #: conditions_db.ConditionsDB instance
        self.db = db
        #: ItemFilter
        self.iovfilter = ItemFilter(args)
        #: number of iovs before payload and revision selection
        self.num_all_iovs = None
        #: Dictionary with past participles
        self.past_dict = {"delete": "deleted", "modify": "modified", "copy": "copied", "create": "created"}

    def add_arguments(self):
        """Add arguments to the parser"""

        self._args.add_argument("tag", metavar="INPUT_TAGNAME",
                                help=f"globaltag for which the the IoVs should be {self.past_dict[self.whichcommand]}")
        if self.whichcommand == "copy":
            self._args.add_argument("output", metavar="OUTPUT_TAGNAME", help="globaltag to which the the IoVs should be copied")
        if self.whichcommand == "modify":
            self._args.add_argument("new_iov", metavar="NEW_IOV", help="New iov to be set to all considered iovs."
                                    " It should be a string with 4 numbers separated by spaces."
                                    " Use * to mark the fields that should not be modified. For example"
                                    " if 7 0 * * is given the iov (7, 1, 9, 42) will become (7 0 9 42).")
        self._args.add_argument("--iov-id", default=None, type=int,
                                help="IoVid of the iov to be considered")
        self._args.add_argument(
            "--iov-pattern",
            default=None,
            help="whitespace-separated string with pattern of the iov to be considered. "
            " Use * to mark the fields that shold be ignored. Valid patterns are 0 0 -1 -1"
            " (a very specific IoV),  0 * -1 -1 (any iov that starts in any run of exp 0 and ends exactly in exp -1, run -1)"
            ", * * 3 45 (any Iov ending in exp 3, run 45, regardless from where it starts).")
        self._args.add_argument("--run-range", nargs=4, default=None, type=int,
                                metavar=("FIRST_EXP", "FIRST_RUN", "FINAL_EXP", "FINAL_RUN"),
                                help="Can be four numbers to limit the run range to be considered"
                                " Only iovs overlapping, even partially, with this range will be considered.")
        self._args.add_argument("--fully-contained", action="store_true",
                                help="If given together with ``--run_range`` limit the list of payloads "
                                "to the ones fully contained in the given run range")
        if self.whichcommand == "copy":
            self._args.add_argument("--set-run-range", action="store_true",
                                    help="If given together with ``--run_range`` modify the interval of validity"
                                    " of partially overlapping iovs to be fully contained in the given run range")
        self.iovfilter.add_arguments("payloads")
        self._args.add_argument("--revision", metavar='revision', type=int,
                                help="Specify the revision of the payload to be removed")
        self._args.add_argument("--dry-run", help="Don't do anything, just print what would be done",
                                action="store_true", default=False)
        if self.whichcommand == "copy":
            self._args.add_argument("--replace", help="Modify the iovs in the output tag to avoid overlaps",
                                    action="store_true", default=False)
        self._args.add_argument("-j", type=int, default=10, dest="nprocess",
                                help="Number of concurrent threads to use.")

    def get_iovs(self):
        """Get the iovs already filtered"""
        if not self.iovfilter.check_arguments():
            B2ERROR("Issue with arguments")

        all_iovs = self.db.get_all_iovs(
            self._args.tag,
            run_range=self._args.run_range,
            fully_contained=self._args.fully_contained,
            message=str(self.iovfilter))
        self.num_all_iovs = len(all_iovs)
        iovs_to_return = []
        for iov in all_iovs:
            if self._args.iov_id and iov.iov_id != self._args.iov_id:
                continue
            if not self.iovfilter.check(iov.name):
                continue
            if self._args.iov_pattern and not fnmatch("{} {} {} {}".format(*iov.iov),
                                                      "{} {} {} {}".format(*self._args.iov_pattern.split())):
                continue
            if self._args.revision and iov.revision != self._args.revision:
                continue
            iovs_to_return.append(iov)
        return iovs_to_return

    def modify_db(self, func, func_args, whichcommand=None):
        """Modify the database using multithreading"""
        if whichcommand is None:
            whichcommand = self.whichcommand
        try:
            with ThreadPoolExecutor(max_workers=self._args.nprocess) as pool:
                start = time.monotonic()
                for iov_num, _ in enumerate(pool.map(func, func_args), 1):
                    eta = (time.monotonic() - start) / iov_num * (len(func_args) - iov_num)
                    B2INFO(f"{iov_num}/{len(func_args)} iovs {self.past_dict[whichcommand]}, ETA: {eta:.1f} seconds")
        except RuntimeError:
            B2ERROR(f"Not all iovs could be {self.past_dict[whichcommand]}. This could be a server/network problem "
                    "or the destination globaltag was not writeable.")
            raise


def command_iovs_delete(args, db=None):
    """
    Delete iovs from a globaltag

    This command allows to delete the iovs from a globaltags, optionally limiting the iovs to be deleted to those
    of a specific payload, revision, IoVid or run range.
    """
    command_iovs_helper = CommandIoVsHelper("delete", args, db)
    if db is None:
        command_iovs_helper.add_arguments()
        return

    iovs_to_delete = command_iovs_helper.get_iovs()

    table = [[i.iov_id, i.name, i.revision] + list(i.iov) for i in iovs_to_delete]
    table.insert(0, ["IovId", "Name", "Rev", "First Exp", "First Run", "Final Exp", "Final Run"])
    columns = [9, "+", 6, 6, 6, 6, 6]

    if command_iovs_helper.num_all_iovs == len(iovs_to_delete) and args.run_range is None:
        B2WARNING(f"All the iovs in the globaltag {args.tag} will be deleted!")
        gt_check = input('Please enter the global tag name again to confirm this action: ')
        if gt_check != args.tag:
            B2ERROR('global tag names do not match.')
            return 1

    B2INFO("Deleting the following iovs")
    pretty_print_table(table, columns)

    if not args.dry_run:
        set_cdb_authentication_token(db, args.auth_token)
        try:
            command_iovs_helper.modify_db(db.delete_iov, [i.iov_id for i in iovs_to_delete])
        except RuntimeError:
            return 1

    return 0


def command_iovs_copy(args, db=None):
    """
    Copy iovs from a globaltag to another one

    This command allows to copy the iovs from a globaltags to another one, optionally limiting
    the iovs to be copied to those of a specific payload, revision, IoV id or run range.
    """
    command_iovs_helper = CommandIoVsHelper("copy", args, db)
    if db is None:
        command_iovs_helper.add_arguments()
        return

    iovs_to_copy = command_iovs_helper.get_iovs()

    # make sure that there are no overlaps in the iovs to copy and set run range if needed
    by_name = defaultdict(lambda: IoVSet(allow_overlaps=False))
    for iov in iovs_to_copy:
        if args.run_range and args.set_run_range:
            iov.iov = (IntervalOfValidity(*iov.iov) & IntervalOfValidity(*args.run_range)).tuple
        try:
            by_name[iov.name].add(iov.iov)
        except ValueError as e:
            B2ERROR(f"Overlap for payload {iov.name} r{iov.revision}: {e}")
            return 1
    del by_name

    # make sure output tag exists
    output_id = db.get_globalTagInfo(args.output)
    if output_id is None:
        B2ERROR("Output globaltag doesn't exist. Please create it first with a proper description")
        return False
    output_id = output_id["globalTagId"]

    # get iovs already present in output global tag
    iovs_output = db.get_all_iovs(args.output)
    iovs_output.sort(key=lambda i: i.iov[:2])
    payloads_output = defaultdict(list)
    for iov in iovs_output:
        iov.newiov = IoVSet([IntervalOfValidity(*iov.iov)])
        payloads_output[iov.name].append(iov)

    # remove from iovs to copy those that are already present in the output global tag
    iovs_in_output = [(i.checksum, i.iov) for i in iovs_output]
    iovs_to_copy = [i for i in iovs_to_copy if (i.checksum, i.iov) not in iovs_in_output]
    del iovs_in_output

    iovs_not_to_copy = []

    # readjust the iovs already present if they overlap
    # with the ones to be copied
    for iov_to_copy in iovs_to_copy:
        iov_ = IntervalOfValidity(*iov_to_copy.iov)
        still_to_copy = True
        for iov_present in payloads_output[iov_to_copy.name]:
            if iov_present.newiov & iov_:
                if still_to_copy and iov_to_copy.checksum == iov_present.checksum:
                    iov_present.newiov.add(iov_, allow_overlaps=True)
                    iovs_not_to_copy.append(iov_to_copy)
                    still_to_copy = False
                else:
                    iov_present.newiov -= iov_

    iovs_to_delete = []
    iovs_to_modify = []
    iovs_to_create = []

    # Fill lists of iovs to be deleted, modified or added
    for iov_present in sum(payloads_output.values(), []):
        # iov_present was replaced by copied iovs so it should be deleted
        if len(iov_present.newiov) == 0:
            iovs_to_delete.append(iov_present)
        # iov_present was partially overlapping by copied iovs so it should be modified
        elif len(iov_present.newiov) == 1:
            if not list(iov_present.newiov)[0].tuple == iov_present.iov:
                iovs_to_modify.append((iov_present, list(iov_present.newiov)[0].tuple))
        # iov_present was split by copied iovs so its first iov should be modified and new iovs should be created
        else:
            newiovs = list(iov_present.newiov)
            iovs_to_modify.append((iov_present, newiovs[0].tuple))
            for newiov in newiovs[1:]:
                iovs_to_create.append((iov_present, newiov.tuple))

    # If I fix the overlaps do not copy iovs that can be obtained extending iovs already present
    if args.replace:
        iovs_to_copy = [i for i in iovs_to_copy if i not in iovs_not_to_copy]

    # Print nice table
    table = [[i.iov_id, i.name, i.revision] + list(i.iov) for i in iovs_to_copy]
    table.insert(0, ["IovId", "Name", "Rev", "First Exp", "First Run", "Final Exp", "Final Run"])
    columns = [9, "+", 6, 6, 6, 6, 6]
    B2INFO(f"Copying the following iovs to {args.output}")
    pretty_print_table(table, columns)

    if (iovs_to_delete or iovs_to_modify or iovs_to_create) and not args.replace:
        B2WARNING("Inserting the iovs will create overlaps,"
                  f" to avoid them the changes below should be implemented on {args.tag}")

    if iovs_to_delete:
        B2WARNING("iovs to be deleted")
        # Sort by payload name and iov for nicer logs
        iovs_to_delete.sort(key=lambda x: x.iov[:2])
        iovs_to_delete.sort(key=lambda x: x.name)
        # Print nice table
        table = [[i.iov_id, i.name, i.revision] + list(i.iov) for i in iovs_to_delete]
        table.insert(0, ["IovId", "Name", "Rev", "First Exp", "First Run", "Final Exp", "Final Run"])
        columns = [9, "+", 6, 6, 6, 6, 6]
        pretty_print_table(table, columns)
    if iovs_to_modify:
        B2WARNING("iovs to be modified")
        # Sort by payload name and iov for nicer logs
        iovs_to_modify.sort(key=lambda x: x[0].iov[:2])
        iovs_to_modify.sort(key=lambda x: x[0].name)
        # Print nice table
        table = [[i[0].iov_id, i[0].name, i[0].revision] + list(i[0].iov) + list(i[1]) for i in iovs_to_modify]
        table.insert(0, ["IovId", "Name", "Rev", "Old First Exp", "Old First Run", "Old Final Exp", "Old Final Run",
                     "New First Exp", "New First Run", "New Final Exp", "New Final Run"])
        columns = [9, "+", 6, 6, 6, 6, 6, 6, 6, 6, 6]
        pretty_print_table(table, columns)
    if iovs_to_create:
        B2WARNING("iovs to be created")
        # Sort by payload name and iov for nicer logs
        iovs_to_create.sort(key=lambda x: x[1][:2])
        iovs_to_create.sort(key=lambda x: x[0].name)
        # Print nice table
        table = [[i[0].name, i[0].revision] + list(i[1]) for i in iovs_to_create]
        table.insert(0, ["Name", "Rev", "First Exp", "First Run", "Final Exp", "Final Run"])
        columns = ["+", 6, 6, 6, 6, 6]
        pretty_print_table(table, columns)

    if (iovs_to_delete or iovs_to_modify or iovs_to_create) and not args.replace:
        B2WARNING("To apply them use the option --replace")
        B2WARNING("If instead you want these overlaps to be present enter"
                  " the output global tag name again to confirm this action: ")
        gt_check = input()
        if gt_check != args.output:
            B2ERROR("global tag names do not match.")
            return 1

    if not args.dry_run:
        set_cdb_authentication_token(db, args.auth_token)
        try:
            command_iovs_helper.modify_db(lambda args: db.create_iov(output_id, *args),
                                          [(i.payload_id, *i.iov) for i in iovs_to_copy])
            if args.replace:
                command_iovs_helper.modify_db(db.delete_iov, [i.iov_id for i in iovs_to_delete], "delete")
                command_iovs_helper.modify_db(lambda args: db.modify_iov(*args),
                                              [(i[0].iov_id, *i[1]) for i in iovs_to_modify], "modify")
                command_iovs_helper.modify_db(lambda args: db.create_iov(output_id, *args),
                                              [(i[0].payload_id, *i[1]) for i in iovs_to_create], "create")
        except RuntimeError:
            return 1

    return 0


def command_iovs_modify(args, db=None):
    """
    Modify iovs from a globaltag

    This command allows to modify the iovs from a globaltags, optionally limiting the iovs to be modified to those
    of a specific payload, revision, IoV id or run range.
    """
    command_iovs_helper = CommandIoVsHelper("modify", args, db)
    if db is None:
        command_iovs_helper.add_arguments()
        return

    new_iov = args.new_iov.split()
    # Transform integer values of iov in int and keep '*' as strings. Raise error if something different is given.
    for i in range(len(new_iov)):
        try:
            new_iov[i] = int(new_iov[i])
        except ValueError:
            if new_iov[i] != '*':
                raise ValueError(f"Invalid IOV value: {new_iov[i]} should be an integer or '*'")

    iovs_to_modify = []
    for iov in command_iovs_helper.get_iovs():
        new_iow_ = new_iov.copy()
        for i in range(len(new_iow_)):
            if new_iow_[i] == '*':
                new_iow_[i] = iov.iov[i]
        iovs_to_modify.append((iov, new_iow_))

    table = [[i[0].iov_id, i[0].name, i[0].revision] + list(i[0].iov) + list(i[1]) for i in iovs_to_modify]
    table.insert(0, ["IovId", "Name", "Rev", "Old First Exp", "Old First Run", "Old Final Exp", "Old Final Run",
                     "New First Exp", "New First Run", "New Final Exp", "New Final Run"])
    columns = [9, "+", 6, 6, 6, 6, 6, 6, 6, 6, 6]

    B2INFO("Changing the following iovs")
    pretty_print_table(table, columns)

    if not args.dry_run:
        set_cdb_authentication_token(db, args.auth_token)
        try:
            command_iovs_helper.modify_db(lambda args: db.modify_iov(*args),
                                          [(i[0].iov_id, *i[1]) for i in iovs_to_modify])
        except RuntimeError:
            return 1

    return 0

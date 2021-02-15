import functools
from concurrent.futures import ProcessPoolExecutor
from collections import defaultdict
from basf2 import B2INFO, B2ERROR, B2WARNING, LogPythonInterface  # noqa
from basf2.utils import pretty_print_table
from terminal_utils import Pager
from conditions_db.iov import IoVSet, IntervalOfValidity
from conditions_db.runningupdate import RunningTagUpdater, RunningTagUpdaterError, RunningTagUpdateMode


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
            B2ERROR(f"Overlap for payload {payload.name}: {e}")

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
        db.create_iov(globaltag_id, payload.payload_id, *iov.tuple)


def command_tag_merge(args, db=None):
    """
    Merge a list of globaltags in the order they are given.

    This command allows to merge a number of globaltags into a single globaltag.
    Payloads from later globaltags in the list of arguments are used to fill gaps
    present in earlier globaltags.

    The result is equivalent to having multiple globaltags setup in the conditions
    access for basf2 (highest priority goes first).

    Warning:
      The order of the globaltags is highest priority first, so payloads from
      globaltags earlier on the command line will be taken with before globaltags
      from later tags.

      This command requires that all globaltags are overlap free.

    For each globaltag in the list we copy all payloads to the output globaltag
    if there is no payload of that name valid for the given interval of validity
    in any previous globaltags in the list.

    If the payload overlaps partially with a payload from a previous globaltag
    in the list the interval of validity is shortened (and possibly split) to
    not overlap but to just fill the gaps.

    For example:

    globaltag A contains ::

        payload1, rev 2, valid from 1,0 to 1,10
        payload1, rev 3, valid from 1,20 to 1,22
        payload2, rev 1, valid from 1,0 to 1,-1

    globaltag B contains ::

        payload1, rev 1, valid from 1,1 to 1,30
        payload2, rev 2, valid from 0,1 to 1,20

    Then running ``b2conditionsdb tag merge -o output A B``, the output globaltag
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
        args.add_argument("-j", type=int, default=1, dest="nprocess",
                          help="Number of concurrent processes to use for "
                          "creating payloads into the output globaltag.")
        return

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
            # get all payload information objects with their iovs already merged to iovset instances
            payloads = get_all_iovsets(db.get_all_iovs(tag), args.run_range)
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
        B2INFO(f'Now copying the payloads into {args.output}...')
        create_iov = functools.partial(create_iov_wrapper, db, output_id)
        with ProcessPoolExecutor(max_workers=args.nprocess) as pool:
            pool.map(create_iov, final)

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

    Example:

        running tag contains ::

            payload1, rev 1, valid from 0,1 to 1,0
            payload1, rev 2, valid from 1,1 to -1,-1
            payload2, rev 1, valid from 0,1 to -1,-1
            payload3, rev 1, valid from 0,1 to 1,0
            payload4, rev 1, valid from 0,1 to -1,-1
            payload5, rev 1, valid from 0,1 to -1,-1

        staging tag contains ::

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

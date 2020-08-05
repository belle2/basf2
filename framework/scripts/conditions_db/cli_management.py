from conditions_db.iov import IoVSet, IntervalOfValidity
from basf2 import B2INFO, B2ERROR, B2FATAL, LogPythonInterface
from collections import defaultdict
from basf2.utils import pretty_print_table
from terminal_utils import Pager
from concurrent.futures import ProcessPoolExecutor
from itertools import repeat


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
      This command requires all globaltags are overlap free.

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
        payload2, rev 2, valid ftom 0,1 to 0,-1
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

    .. versionadded:: release-05-00-00
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
                          help="Number of concurrent processes to use for creating payloads into the output globaltag.")
        return

    # prepare some colors for easy distinction of source tag
    support_color = LogPythonInterface.terminal_supports_colors()
    if support_color:
        colors = "\x1b[32m \x1b[34m \x1b[35m \x1b[31m".split()
        colors = {tag: color for tag, color in zip(args.globaltag, colors)}

    def color_row(row, widths, line):
        """Color the lines depending on which globaltag the payload comes from"""
        if not support_color:
            return line
        begin = colors.get(row[-1], "")
        end = '\x1b[0m'
        return begin + line + end

    with Pager(f"Result of merging globaltags", True):
        # make sure output tag exists
        output_id = db.get_globalTagInfo(args.output)
        if output_id is None:
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
        with ProcessPoolExecutor(max_workers=args.nprocess) as pool:
            pool.map(create_iov_wrapper, repeat(db, len(final)), repeat(output_id, len(final)), final)

    return 0

#!/usr/bin/env python3

"""
This module contains some classes to check the possibility and calculate the
necessary updates for a running globaltag
"""

from enum import Enum
from collections import defaultdict
from basf2 import B2ERROR, B2WARNING, B2INFO, B2WARNING  # noqa
from . import ConditionsDB
from .iov import IntervalOfValidity, IoVSet


class RunningTagUpdateMode(Enum):
    """Define the different modes for the update of a running tag"""
    #: Allow multiple validity for payloads in the staging tag which don't have
    #  to start exactly at the first valid run but have to cover the whole area
    #  from their first validity to infinity
    STRICT = 1
    #: Similar to strict but allow validity to not extend to infinity
    ALLOW_CLOSED = 2
    #: Similar to strict but automatically extend validity to infinity
    FIX_CLOSED = 3
    #: Only allow trivial (0,0,-1,-1) validities in the staging tag and only one
    #  validity per payload name
    SIMPLE = 4
    #: Replace full state of the globaltag. Similar to ALLOW_CLOSED but any
    #  payload present and open in the running tag that is **not** part of the
    #  staging tag will be closed. After the update the new valid payloads will
    #  be exactly the ones in the staging tag
    FULL_REPLACEMENT = 6


class RunningTagUpdaterError(Exception):
    """
    Errors raised when trying to update the running globaltag. Can have extra
    variables in `extra_vars` to be shown to the user for additional information
    """
    #: Initialize the class.
    def __init__(self, description, **extra_vars):
        super().__init__(description)
        #: extra keyword arguments given to the exception constructor
        self.extra_vars = extra_vars


class RunningTagUpdater:
    """
    Calculate and apply the necessary changes to update a running globaltag

    For this we take two globaltags: the running one and a staging one
    containing all the payloads and iovs to be added to the running tag. We then

    1. Make sure they are in the correct states (RUNNING and VALIDATED)
    2. Make sure all payloads in the running tag start and end (except for open
       iovs) before the given ``valid_from`` run
    3. Make sure the staging tag is overlap and gap free
    4. Make all payloads in staging start at either 0,0 or on/after the given
       ``valid_from`` run
    5. Make sure all payloads in staging are unbound unless the mode is
       ``ALLOW_CLOSED`` or ``FIX_CLOSED``. In case of ``FIX_CLOSED`` extend the
       last iov to infinity
    6. Close all payloads to be updated in the running tag that are open just
       before the validity in the staging tag.
    """
    def __init__(self, db, running, staging, valid_from, mode, dry_run=False):
        """Initialize the class

        Arguments:
            db (ConditionsDB): reference to the database object
            running (str): name of the running tag
            stagin (str): name of the staging tag
            valid_from (tuple(int,int)): first valid exp,run
            mode (RunningTagUpdateMode): the mode of the update
            dry_run (bool): If true only check, don't do anything.
                But be more lenient with globaltag state of staging.
        """
        #: Reference to the database object to use for queries
        self._db = db

        # make sure the valid_from is a tuple of two ints
        try:
            valid_from = tuple(map(int, valid_from))
            if len(valid_from) != 2:
                raise ValueError("exp,run number needs to have two elements")
        except Exception as e:
            raise RunningTagUpdaterError("No first valid run for the update specified", error=str(e)) from e

        #: If we're in dry run mode be less critical about globaltag states
        #  (just show warnings) but refuse to do any actual update
        self._dry_run = dry_run
        #: First valid run for the update
        self._valid_from = valid_from

        #: True if we want to allow payloads in the staging tag to be closed,
        #  for example when retireing a payload
        self._mode = mode
        #: Do we allow closed iovs in staging?
        self._allow_closed = mode in (RunningTagUpdateMode.ALLOW_CLOSED, RunningTagUpdateMode.FIX_CLOSED,
                                      RunningTagUpdateMode.FULL_REPLACEMENT)
        #: Do we want to automatically open closed iovs?
        self._fix_closed = (mode == RunningTagUpdateMode.FIX_CLOSED)
        #: Dictionary mapping payload names in the staging tag to the coverage
        #  they have in the staging tag, filled by check_staging_tag
        self._staging_coverage = None
        #: Operations for the update, filled by calculate_update()
        self._operations = None

        #: Globaltag information for the running tag
        self._running_info = db.get_globalTagInfo(running)
        #: Globaltag information for the staging tag
        self._staging_info = db.get_globalTagInfo(staging)
        # make sure the tags can be found and have the correct state
        self._check_state(running, self._running_info, "RUNNING")
        self._check_state(staging, self._staging_info, "VALIDATED")

        # Get the actual payloads
        #: Payloads currently in the running tag
        self._running_payloads = db.get_all_iovs(self._running_info['name'])
        #: Payloads currently in the staging tag
        self._staging_payloads = db.get_all_iovs(self._staging_info['name'])
        #: First iov per payload name in staging to not close and open the same revision
        self._staging_first_iovs = {}

    def _check_state(self, tagname, taginfo, required):
        """Check the state of a globaltag given the tag information object returned by the database

        1) that it's found and
        2) that it has the same state as in ``required``

        Parameters:
            tagname: name of the tag for error messages
            taginfo: tag information returned from the database, None if the tag could not be found
            required: required state for the tag.

        Raises:
            an `RunningTagUpdaterError` if any condition is not fulfilled
        """
        if taginfo is None:
            raise RunningTagUpdaterError(f"Globaltag '{tagname}' cannot be found")
        state = taginfo['globalTagStatus']['name'].upper()
        if state != required.upper():
            if self._dry_run:
                B2WARNING(f"Globaltag '{tagname}' not in {required.upper()} state, continuing to display changes")
                return
            raise RunningTagUpdaterError(f"Globaltag '{tagname}' not in {required.upper()} state", state=state)

    def _check_all(self):
        """Run all necessary checks on all globaltags"""
        # And check both tags
        self._check_running_tag(self._running_info['name'], self._running_payloads)
        # and check the staging tag
        if self._mode == RunningTagUpdateMode.SIMPLE:
            # the simple mode is so much simpler that a different method is best
            self._check_staging_tag_simple(self._staging_info['name'], self._staging_payloads)
        else:
            # all other modes are covered here
            self._check_staging_tag(self._staging_info['name'], self._staging_payloads)

    def _check_running_tag(self, tagname, payloads):
        """
        Check that all payloads in the running tag start and end (or are open)
        before the first valid run for the update
        """
        errors = {
            "payloads start after first valid run": 0,
            "payloads end after first valid run": 0
        }
        earliest_valid_from = (0, 0)
        for p in payloads:
            iov = IntervalOfValidity(p.iov)
            # starting of a validity is simple ... it needs to be below the first valid run
            if iov.first >= self._valid_from:
                B2ERROR(f"Payload in running tag '{tagname}' starts after first valid run",
                        payload=p.name, iov=p.iov, **{"first valid run": self._valid_from})
                errors["payloads start after first valid run"] += 1
            # end of a validity only matters for closed iovs, open iovs can get clipped
            elif iov.final != IntervalOfValidity.always().final and iov.final >= self._valid_from:
                B2ERROR(f"Payload in running tag '{tagname}' ends after first valid run",
                        payload=p.name, iov=p.iov, **{"first valid run": self._valid_from})
                errors["payloads end after first valid run"] += 1

            earliest_valid_from = max(earliest_valid_from, iov.first)
            if iov.final != IntervalOfValidity.always().final:
                earliest_valid_from = max(earliest_valid_from, iov.final)

        if self._dry_run:
            B2INFO("Earliest possible update of the running tag would be exp "
                   f"{earliest_valid_from[0]}, run {earliest_valid_from[1] + 1}")

        # show errors if we have any ...
        if any(errors.values()):
            raise RunningTagUpdaterError("Given first valid run conflicts with "
                                         f"running tag '{tagname}'", **errors)

    def _check_staging_tag_simple(self, tagname, payloads):
        """
        Extra simple case where we want to have a very simple staging tag just
        consisting of (0,0,-1,-1) iovs, one per payload
        """
        # This is the easy case: Make sure **ALL** iovs are the same and we only
        # have one per payload name.
        payload_names = set()
        errors = {"duplicate payloads": 0, "wrong validity": 0}
        for p in payloads:
            if p.name in payload_names:
                B2ERROR(f"Duplicate payload in staging tag '{tagname}'", name=p.name)
                errors["duplicate payloads"] += 1
            payload_names.add(p.name)
            if p.iov != (0, 0, -1, -1):
                errors["wrong validity"] += 1
                B2ERROR(f"Wrong validity for payload in staging tag '{tagname}'", name=p.name, validity=p.iov)

            # trivial, only one payload per name to remember so it's always the first
            self._staging_first_iovs[p.name] = p

        # how many errors did we have?
        if any(errors.values()):
            raise RunningTagUpdaterError(f"Staging tag '{tagname}' not fit for update in simple mode", **errors)

        # everything is fine, set the coverage to everything for all payloads ...
        always = IoVSet([IntervalOfValidity.always()])
        self._staging_coverage = {name: always for name in payload_names}

    def _check_staging_tag(self, tagname, payloads):
        """
        Check if the staging tag is
            1. overlap free
            2. gap free
            3. all payloads are open (unless the mode allows closed payloads)
            4. all payloads start at 0,0 or after the first valid run for the update

        Arguments:
            tagname (str): Name of the globaltag for error messages
            payloads (list(conditions_db.PayloadInformation)): List of payloads in the tag
        """
        # coverage for all iovs for a payload name not starting at (0,0) as those
        # will be adjusted to first valid run
        explicit_coverage = defaultdict(IoVSet)
        # full coverage of the payload to check for overlaps and gaps and later
        # use it to correctly close open iovs in the running tag
        full_coverage = defaultdict(IoVSet)
        # if we want to open iovs we need to know which was the latest
        latest_iov = {}
        # error dictionary to return to caller
        errors = {'overlaps': 0, 'gaps': 0, 'starts too early': 0, 'closed payloads': 0}
        # go through all payloads and form the union of all their validities ...
        # and check for overlaps
        for p in payloads:
            iov = IntervalOfValidity(p.iov)
            try:
                # add the iov to the
                full_coverage[p.name].add(iov, allow_overlaps=False)
            except ValueError as e:
                B2ERROR(f"Overlap in globaltag '{tagname}'", payload=p.name, overlap=e)
                errors['overlaps'] += 1
                # now add it anyways to check for more overlaps with the other
                # iovs in the tag just to give a complete list of errors
                full_coverage[p.name].add(iov, allow_overlaps=True)

            # add all iovs not starting at 0,0 to the explicit coverage of this payload
            if iov.first != (0, 0):
                explicit_coverage[p.name].add(iov, allow_overlaps=True)

            # do we need to open iovs? If so remember the latest iov for each payload
            if self._fix_closed:
                prev = latest_iov.get(p.name, None)
                if prev is None:
                    latest_iov[p.name] = p
                else:
                    latest_iov[p.name] = max(p, prev)

            # remember the first iov of each payload to extend what's in running if they match
            first = self._staging_first_iovs.get(p.name, None)
            if first is None:
                self._staging_first_iovs[p.name] = p
            else:
                self._staging_first_iovs[p.name] = min(p, first)

        # Ok, now check for all payloads if the resulting iov is a single one or multiple,
        # aka having gaps. In that case print the gaps
        for name, iovs in full_coverage.items():
            if len(iovs) > 1:
                B2ERROR(f"Gap in globaltag '{tagname}'", payload=name, gaps=iovs.gaps)
                errors['gaps'] += len(iovs) - 1
            # Also, make sure the iovs for the payloads are infinite and either
            # raise errors or at least show a warning
            if iovs.final != IntervalOfValidity.always().final:
                log_func = B2WARNING
                if not self._allow_closed:
                    log_func = B2ERROR
                    errors['closed payloads'] += 1
                log_func(f"Payload in globaltag '{tagname}' not open ended",
                         payload=name, **{"final run": iovs.final})

        # No gaps, no overlaps, but do we start at the given valid_from run?
        for name, iovs in explicit_coverage.items():
            if iovs.first < self._valid_from:
                B2ERROR(f"Payload in globaltag '{tagname}' starts before the given first valid run",
                        payload=name, **{"actual start validity": iovs.first,
                                         "expected start validity": self._valid_from})
                errors['starts too early'] += 1

        # Do we want to open iovs?
        if self._fix_closed:
            # Then do so ...
            for payload in latest_iov.values():
                if payload.iov[2:] != (-1, -1):
                    B2INFO("Extending closed iov to infinity", name=payload.name,
                           **{"old iov": payload.iov})
                    payload.iov = payload.iov[:2] + (-1, -1)
                    full_coverage[payload.name].add(payload.iov, allow_overlaps=True)

        # Any errors?
        if any(errors.values()):
            raise RunningTagUpdaterError(f"Staging tag '{tagname}' not fit for update", **errors)

        # No errors, great, remember the coverages for all payloads to select the
        # proper payloads to be closed in the running tag and where to close them
        self._staging_coverage = dict(full_coverage)

    def calculate_update(self):
        """
        Calculate the operations needed to merge staging into the running base tag
        """
        # Check all tags and payloads
        self._check_all()
        # Ok, all checks done ...
        valid_range = IntervalOfValidity(*(self._valid_from + (-1, -1)))
        operations = []
        # we only need to close payloads that are present in staging ... unless
        # we run in FULL mode in which we close everything
        for p in self._running_payloads:
            # so check if we have a coverage in the staging tag
            staging = self._staging_coverage.get(p.name, None)
            if p.iov[2:] == (-1, -1):
                if staging is None:
                    # payload not present in staging tag. Ignore unless we do full replacement
                    if not self._mode == RunningTagUpdateMode.FULL_REPLACEMENT:
                        continue
                    staging_range = valid_range
                else:
                    # extend the staging to infinity to make sure we close existing payloads correctly
                    staging_range = IntervalOfValidity(*(staging.first + (-1, -1))) & valid_range

                # if the first payload didn't change revision we don't need to make
                # a new iov but can just extend the existing one
                first_iov = self._staging_first_iovs.get(p.name, None)
                if first_iov is not None and first_iov.revision == p.revision:
                    staging_range -= IntervalOfValidity(first_iov.iov)
                    self._staging_payloads.remove(first_iov)
                    # there's a chance this is empty now
                    if not staging_range:
                        continue
                # close the existing iov before the range covered in staging
                p.iov = (IntervalOfValidity(p.iov) - staging_range).tuple
                # and mark the iov for closing
                operations.append(["CLOSE", p])
        # and all payloads that need adjusting in staging
        for p in self._staging_payloads:
            # clip them to the valid range ... which only affects iovs starting
            # from 0,0. Everything else would have raised an error
            p.iov = (IntervalOfValidity(p.iov) & valid_range).tuple
            # and add them to the list
            operations.append(["CREATE", p])

        # remember the operations in case we want to apply them
        self._operations = operations
        return self._operations

    def apply_update(self):
        """Apply a previously calculated update to the globaltag

        Warning:
            This action cannot be undone, only call it after checking the
            operations returned by the calculation of the update
        """
        if self._dry_run:
            raise RunningTagUpdaterError("Called in dry-run mode, refusing to cooperate")

        if self._operations is None:
            raise RunningTagUpdaterError("Update needs to be calculated first")

        operations = []
        for op, payload in self._operations:
            if op == "CLOSE":
                operations.append({"operation": "MODIFY", "data": [payload.iov_id] + list(payload.iov[2:])})
            elif op == "CREATE":
                operations.append({"operation": "CREATE", "data": [payload.payload_id] + list(payload.iov)})
            else:
                raise RunningTagUpdaterError(f"Unknown operation type: {op}")

        if not operations:
            return

        tag = self._running_info['name']
        try:
            self._db.request("POST", f"/globalTagPayload/{tag}/updateRunningPayloads",
                             f"updating running tag {tag}", json=operations)
        except ConditionsDB.RequestError as e:
            raise RunningTagUpdaterError(f"Cannot update running tag {tag}", error=e) from e

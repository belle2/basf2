#!/usr/bin/env python3

"""
Script to prepare a local database for HLT validation.

This is more complicated as we would like because performance of HLT depends on
the payloads matching to the given run but we also want to valdiate new payloads
and/or new software versions. So as a "best guess" what could work we take with
the following priority

1. any payloads in online that are unlimited.
2. any payloads in online valid for the validation run but no longer unlimited
   *if* a payload with the same name still exists as unlimited in the online tag.
3. any payloads in staging_online

We take all of these and put them in a local database file and download all the
necessary payload files.
"""

import os
import copy
from pathlib import Path
import multiprocessing
import functools
from conditions_db import ConditionsDB
from conditions_db.iov import IntervalOfValidity
from conditions_db.local_metadata import LocalMetadataProvider
from conditions_db.cli_download import download_payload


def update_payload(existing, payload, priority, source):
    # normally we'd just take one but right now there are overlaps in
    # online so better safe than sorry: take highest revision
    update = payload.name not in existing
    if not update:
        other = existing[payload.name]
        update = other.priority < priority or \
            (other.priority == priority and other.revision < payload.revision)

    if update:
        # keep for all runs
        p = copy.copy(payload)
        p.iov = (0, 0, -1, -1)
        # and remember priority gt for debugging
        p.priority = priority
        p.source = source
        existing[p.name] = p


def prepare_globaltag(db, exp, run):
    payloads = {}

    # get the unlimited iovs from online ... and the ones for our particular run
    base = db.get_all_iovs("online")
    existing_names = set()
    for p in base:
        iov = IntervalOfValidity(p.iov)
        if iov.is_open:
            update_payload(payloads, p, 1, "online")
            existing_names.add(p.name)

    for p in base:
        iov = IntervalOfValidity(p.iov)
        if not iov.is_open and iov.contains(exp, run) and p.name in existing_names:
            update_payload(payloads, p, 2, f"e{exp}r{run}")

    # and take everything from staging
    staging = db.get_all_iovs("staging_online")
    for p in staging:
        update_payload(payloads, p, 3, "staging")

    return sorted(payloads.values())


if __name__ == "__main__":
    output_dir = Path("cdb")
    metadata = output_dir / "metadata.sqlite"
    if metadata.exists():
        metadata.unlink()
    else:
        output_dir.mkdir(parents=True, exist_ok=True)

    db = ConditionsDB()

    # check if we have a prepared globaltag to check Also allow overriding this in
    # a bamboo build variable if necessary. If we have a prepared globaltag we don't
    # build our own version but just download it.
    existing_gt = os.environ.get("VALIDATION_GLOBALTAG", "").strip()
    if existing_gt:
        payloads = db.get_all_iovs(existing_gt)
        for p in payloads:
            p.source = existing_gt
    else:
        payloads = prepare_globaltag(db, int(os.environ['VALIDATION_EXP']), int(os.environ['VALIDATION_RUN']))

    # Save under the name "online" in the local database file
    localdb = LocalMetadataProvider(str(metadata), mode="overwrite")
    localdb.add_globaltag(1, "online", "RUNNING", payloads)

    # Download all the payloads
    downloader = downloader = functools.partial(download_payload, db, directory=output_dir)
    with multiprocessing.Pool() as p:
        p.map(downloader, payloads)

    # and print the final result
    maxlen = max(len(p.name) for p in payloads)
    print("Payloads taken: ")
    for p in payloads:
        print(f"{p.name:<{maxlen}s} {p.revision:7d} from {p.source}")

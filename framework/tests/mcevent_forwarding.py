#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import basf2
from b2test_utils import clean_working_directory, safe_process, show_only_errors
from ROOT import Belle2
import subprocess
import json


def check_mc_events(outfile, expect):
    """Check if the number of MC events reported by the file metadata is what whe expect"""
    try:
        metadata = subprocess.check_output(["b2file-metadata-show", "--json", outfile])
        mcEvents = json.loads(metadata)["mcEvents"]
    except Exception as e:
        basf2.B2ERROR(f"Cannot determine number of MC events: {e}")
        return False

    if mcEvents != expect:
        basf2.B2ERROR(f"{outfile} MCEvents not correct: got {mcEvents}, expected {expect}")
        return False
    else:
        basf2.B2INFO(f"{outfile}: MCEvents=={expect} Ok")
        return True


def run_rootio(outfile, infiles, expect, *, N=0, **input_kwargs):
    """
    Run RootInput/RootOutput and check that the number of events is what we want

    Arguments:
        outfile (str): filename for the output
        infiles (list): list of input filenames
        expect (int): numer of MC events we expect in the final file
        N (int): pass to process() as second argument to limit the number of events

    All other arguments are passed to RootInput as module parameters
    """

    global failures
    path = basf2.create_path()
    path.add_module("RootInput", inputFileNames=infiles, **input_kwargs)
    path.add_module("RootOutput", outputFileName=outfile)
    # zero counters so we can continue processing even if we already reported
    # errors before
    basf2.logging.zero_counters()
    with show_only_errors():
        safe_process(path, N)

    result = check_mc_events(outfile, expect)
    failures += 0 if result else 1
    return result


# no summary and load dictionaries early
basf2.logging.enable_summary(False)
Belle2.Environment.Instance().setNumberEventsOverride(0)

# so let's go in an empty directory
with clean_working_directory():
    # and create a few files with some empty events
    all_files = []
    event_counts = []
    failures = 0
    for i in range(1, 16, 7):
        all_files.append(f"events-{i:03d}.root")
        event_counts.append(i)
        generate = basf2.create_path()
        generate.add_module("EventInfoSetter", evtNumList=i)
        generate.add_module("RootOutput", outputFileName=all_files[-1])
        with show_only_errors():
            safe_process(generate)

    # now for either one or all of those files run a number of checks:
    for name, total, filenames, sequence in [
        ("single", event_counts[-1], all_files[-1:], ["1:3"]),
        ("total", sum(event_counts), all_files, ["0", "1:2", "2-"]),
    ]:
        # simple processing should forward MC events
        run_rootio(f"{name}.root", filenames, total)
        # but any skipping should not
        run_rootio(f"{name}-skip1.root", filenames, 0, skipNEvents=1)
        run_rootio(f"{name}-skipTo.root", filenames, 0, skipToEvent=[0, 0, 1])
        run_rootio(f"{name}-sequence.root", filenames, 0, entrySequences=sequence)
        # and limiting the number of events should only forward if all events
        # are processed
        for N in [1, total-1, total, 100000]:
            Belle2.Environment.Instance().setNumberEventsOverride(N)
            run_rootio(f"{name}-basf2-n-{N}.root", filenames, 0 if N < total else total)
            Belle2.Environment.Instance().setNumberEventsOverride(0)
            run_rootio(f"{name}-proc-{N}.root", filenames, 0 if N < total else total, N=N)

    # and if some files are processed multiple times we don't want to forward
    # the number either
    run_rootio(f"duplicate.root", all_files*2, 0)

if failures > 0:
    basf2.B2ERROR(f"{failures} tests failed ...")
    sys.exit(1)

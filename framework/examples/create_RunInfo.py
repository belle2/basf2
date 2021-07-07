#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Simple script to import RunInfo from the DAQ database into conditions
db payloads.
As input you need a file which contains on each line the following
fields separated by whitespace::

exp run runtype StartTime StopTime receivedNevent AcceptedNevent sentNevent runLength
TriggerRate PXDflag SVDflag CDCflag TOPflag ARICHflag ECLflag KLMflag BadRun
"""

import os
import argparse
# make sure ROOT does not steal our command line arguments
from ROOT import PyConfig
#: Tell ROOT to not mangle our command line options
PyConfig.IgnoreCommandLineOptions = True  # noqa
#: And we don't need a gui thread
PyConfig.StartGuiThread = False  # noqa
# now we can import the Belle2 namespace
from ROOT import Belle2
from basf2 import B2FATAL, B2ERROR, B2DEBUG, B2INFO


def get_argument_parser():
    """Function returning the argument parser. Done this way for the automatic
    documentation in sphinx"""
    # we already formatted the docstring in the beginning of the file to look the way we want it: don't
    # reformat it but use it as description
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("filename", help="Filename containing the run information, "
                        "one run per line with the values separated by whitespace")
    return parser


# Doxygen complains about undocumented variables below which are not exported
# ... so hide them on purpose
# @cond this_is_a_main_block_and_not_exported

if __name__ == "__main__":
    parser = get_argument_parser()
    args = parser.parse_args()
    if not os.path.exists(args.filename):
        B2FATAL(f"Input filename {args.filename} does not exist")

    # Number of errors
    errors = 0
    # Number of successfully imported run infos
    imported = 0
    # in reality this should probably not come from a text file but directly
    # from a postgres query in python and skip the intermediate file.
    with open(args.filename) as f:
        for i, line in enumerate(f, 1):
            # ignore empty lines
            line = line.strip()
            if not line:
                continue

            # exp run runtype StartTime StopTime receivedNevent AcceptedNevent
            # sentNevent runLength  TriggerRate PXDflag SVDflag CDCflag TOPflag
            # ARICHflag ECLflag KLMflag BadRun
            # create RunInfo object
            info = Belle2.RunInfo()
            try:
                (
                    exp, run, runtype, starttime, stoptime, receivedNevent, acceptedNevent,
                    sentNevent, runlength, trigger_rate, pxd, svd, cdc, top, arich, ecl,
                    klm, badrun
                ) = line.split()
                info.setExp(int(exp))
                info.setRun(int(run))
                info.setRunType(int(runtype))
                info.setStartTime(int(starttime))
                info.setStopTime(int(stoptime))
                info.setReceivedNevent(int(receivedNevent))
                info.setAcceptedNevent(int(acceptedNevent))
                info.setSentNevent(int(sentNevent))
                info.setRunLength(int(runlength))
                info.setTriggerRate(float(trigger_rate))
                info.setBelle2Detector(*[int(e) for e in (pxd, svd, cdc, top, arich, ecl, klm)])
                info.setBadRun(int(badrun))
                # convert all arguments to an int and pass them on

                B2DEBUG(100, f"Exp: {exp}\tRun: {run}\t RunType: {runtype} ...")
            # seems the line was not formatted correctly so print an error and
            # don't create a payload
            except Exception as e:
                B2ERROR(f"Problem reading line {i}: {e}. Skipping ...")
                errors += 1
                continue

            # finally we have a runinfo object, create payload
            iov = Belle2.IntervalOfValidity(info.getExp(), info.getRun(), info.getExp(),
                                            info.getRun())
            Belle2.Database.Instance().storeData("RunInfo", info, iov)
            imported += 1

    B2INFO(f"Imported {imported} RunInfo objects")
    if errors > 0:
        B2FATAL(f"{errors} errors occured")

# @endcond

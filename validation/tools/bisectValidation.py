#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""

Utility script to be used in conjuction with the git bisect functionality [1] to
search for degredations in validation variables. To use it, you can set the last known
good git commit and the first known bad commit and start the bistecting process.
Git will then checkout commits between the good and the bad and perform a binary search
to arrive at the first bad commit.

To give feed back to git, which is first bad commit, this script either returns the
exit code 0 for good commit, code 125 to ignore the current commit or code 1 for a bad commit.

Information contained within .root files produced by the official validation scripcts or your
custom scripts can be used to distingush between good and bad commits.

As an example, to identfiy the commit which made the tracking efficiency drop below 0.93, the
following snippet can be used:

git bisect good 057a1feb7f6c3a8fe39514ff85680980e730e520
git bisect bad 2b69fa4
git bisect run bisectValidation.py --execute runTrackingEfficiency.sh_explicit
                                   --check-quantity "TrackingValidation.root:hEfficiency[2]:>:0.93"

 Author: The Belle II Collaboration
 Contributors: Thomas Hauth

[1]
http://git-scm.com/docs/git-bisect

"""

import os
import sys
import argparse
import ROOT

import argparse

# this is a 1:1 copy of tracking/scripts/tracking/validation/extract_information_from_tracking_validation_output.py
# which is made necessary at the moment, because the method was recently improved and git bisect might checkout
# older version of this code, where this method is not present.
# us the tracking/scripts version, once the implementation of this function has stabilized


def extract_information_from_file(file_name, results):
    # Read in the given root file and export the information to the results
    root_file = ROOT.TFile(file_name)
    if not root_file.IsOpen():
        return None

    keys = root_file.GetListOfKeys()
    for graph_or_table in keys:
        name = graph_or_table.GetName()
        # As the naming convention is that simple in the validation output, this single line should do the job
        root_obj = root_file.Get(name)

        if "overview_figures_of_merit" in str(name):
            root_obj.GetEntry(0)
            for branch in overview_table.GetListOfBranches():
                branch_name = branch.GetName()
                if branch_name in results:
                    results[branch_name].append(100.0 * float(getattr(root_obj, branch.GetName())))

        # also support histograms
        if root_obj.IsA().GetName() == "TH1F":
            if root_obj.GetName() in results:
                # compute the y average across bins
                # useful for efficiency over <X> plots
                nbinsx = root_obj.GetNbinsX()
                sum = 0.0
                sumZeroSupressed = 0.0
                countZeroSupressed = 0
                for i in range(nbinsx):
                    v = root_obj.GetBinContent(i + 1)
                    sum = sum + v  # from first bin, ignored underflow (i=0) and overflow (i=nbinsx+1) bins
                    if v > 0.0:
                        sumZeroSupressed = sumZeroSupressed + v
                        countZeroSupressed = countZeroSupressed + 1
                meanY = sum / nbinsx
                meanYzeroSupressed = sum / countZeroSupressed

                results[root_obj.GetName()] = (root_obj.GetMean(), meanY, meanYzeroSupressed)

    root_file.Close()

    return results


def parseCheckQuantity(quantString):
    fragments = quantString.split(":")
    if len(fragments) == 4:
        filename = fragments[0].strip()
        quant = (fragments[1].strip('"').strip(), None)
        comp = fragments[2].strip()
        val = fragments[3].strip()

        # parse quantity further ?
        if "[" in quant[0] and "]" in quant[0]:
            splitQuant = filter(lambda x: len(x) > 0, quant[0].replace("[", "]").split("]"))
            spiltQuant = map(lambda x: x.strip(), splitQuant)
            quant = (splitQuant[0], splitQuant[1])

    else:
        print "cannot parse quantity expression " + str(quantString)
        sys.exit(1)

    return (filename, quant, comp, val)

parser = argparse.ArgumentParser(description='Evaluate status of the compile and validation of a specific basf2 revision.')
parser.add_argument('--report-compile-fail', action='store_true',
                    default=False,
                    help='Report a bad revision to git if the code base did not compile. By default, failing compiles are ignored.')
parser.add_argument(
    '--report-execution-fail',
    action='store_true',
    default=False,
    help='Report a bad revision to git if the provided script has an error code != 0. By default, this is reported to git.')
parser.add_argument('--check-quantity', action="append",
                    help='Check for a quantity in validation files')
parser.add_argument('--execute', action="append",
                    help='File to execute after the compile and before the quantity check')

# check quantity format
# 'filename.root:hEfficiency[2]:<:0.95'
args = parser.parse_args()
argsVar = vars(args)

if argsVar["check_quantity"] is None:
    argsVar["check_quantity"] = []

c_parsed = []
for c_string in argsVar["check_quantity"]:
    c_parsed = c_parsed + [parseCheckQuantity(c_string)]

print argsVar
print "Compiling revision ..."

os.system("echo -n 'git commit ' && git rev-parse HEAD")
os.system("echo -n 'SVN revsion ' && git svn find-rev `git rev-parse HEAD`")

exitCode = os.system("scons -j8")
print "Exit code of compile was " + str(exitCode)
if exitCode > 0 and argsVar["report_compile_fail"]:
    sys.exit(125)  # tell git to ignore this failed build

# execute validation
if argsVar["execute"] is None:
    argsVar["execute"] = []

for ex in argsVar["execute"]:
    print "Excuting " + str(ex)
    exitCode = os.system(ex.strip('"'))
    print "Exit code of " + str(ex) + " was " + str(exitCode)

    if exitCode > 0:
        if argsVar["report_execution_fail"]:
            sys.exit(1)  # tell git about this failed run ...
        else:
            sys.exit(125)  # tell git to ignore this failed validation

# perform checks
for c in c_parsed:

    print "Running check on file " + str(c[0]) + " for quantity " + str(c[1]) + " " + str(c[2]) + " " + str(c[3])

    file_name = c[0]

    refObjKey = c[1][0]
    results = {refObjKey: None}
    results = extract_information_from_file(file_name, results)
    print "results " + str(results)
    if results is None:
        print "result file " + str(file_name) + " not found"
        print "Check failed"
        sys.exit(1)

    if results[refObjKey] is None:
        print "could not find quantity " + str(c[1]) + " in file " + str(c[0])
        print "Check failed"
        sys.exit(1)

    if c[1][1] is None:
        valValue = results[refObjKey]
    else:
        subKeyNumber = int(c[1][1])
        valValue = results[refObjKey][subKeyNumber]
    print "Retrieved value " + str(valValue)

    compareResult = None
    if c[2] is ">":
        compareResult = float(valValue) > float(c[3])
    elif c[2] is "<":
        compareResult = float(valValue) < float(c[3])
    else:
        print "Compare operation " + c[2] + " not supported"
        sys.exit(0)

    if not compareResult:
        print "Check failed"
        sys.exit(1)
    else:
        print "Check successful"
        sys.exit(0)

sys.exit(0)

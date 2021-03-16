#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Purpose:
#   Analyze DST [S]ROOT files with a sequence of RPC lookback-window settings and write
#   resulting histograms/scatterplots to ROOT and PDF files.
#
# Prerequisite (on kekcc):
#   Before running this script, type
#     source /cvmfs/belle.cern.ch/tools/b2setup release-03-01-00 <or higher release>
#   then verify that the corresponding proper global tag is used near the end of this script.
#   (Global tags are tabulated at https://confluence.desy.de/display/BI/Global+Tag+%28GT%29+page)
#   The scripts bklmDB.py and EventInspectorLookback.py must be in the same folder as this script.
#
# Usage:
#   basf2 bklm-lookback.py -- -m mode -w # # # -e # -r # -i infilenames -t tagname
#      You need the '--' before these options to tell basf2 that these are options to this script.
#   Required arguments:
#      -i infilename  to specify the full pathname of the input [S]ROOT files, including wildcards (no default)
#      -e #   to specify the experiment number (no default)
#      -r #   to specify the run number (no default)
#      -m mode to specify the lookback-window mode, one of coarse-start, coarse-width, fine-start, fine-width
#      -w # # # to specify the minimum, maximum and step in the lookback-window values
# Optional arguments:
#      -t tagName   to specify the name of conditions-database global tag (no default)
#
# Input:
#   [S]ROOT file written by basf2 (may include multiple folios for one expt/run). For example,
#   /ghi/fs01/belle2/bdata/group/detector/BKLM/localRun/e0008/r00550/debug.0008.00550.HLT\*.f\*.sroot
#   /ghi/fs01/belle2/bdata/Data/Raw/e0008/r00550/sub00/debug.0008.00550.HLT\*.f\*.root
#
# Output:
#   ROOT histogram file named bklmHists-e#r#.root, using the experiment number and run number
#   PDF file named bklmHists-e#r#.pdf, using the experiment number and run number
#

import basf2
import sys
import re
import EventInspectorLookback
import rawdata
from optparse import OptionParser
import glob

parser = OptionParser()
parser.add_option('-i', '--inputfile',
                  dest='infilename', default='',
                  help='Input [S]ROOT filename [no default]')
parser.add_option('-e', '--experiment',
                  dest='eNumber', default='',
                  help='Experiment number [no default]')
parser.add_option('-r', '--run',
                  dest='rNumber', default='',
                  help='Run number [no default]')
parser.add_option('-m', '--mode',
                  dest='mode', default='',
                  help='Lookback-window mode [no default]')
parser.add_option('-w', '--windowValues',
                  dest='window', nargs=3, type="int",
                  help='Lookback-window values (min, max, step) [no default]')
parser.add_option('-t', '--tagName',
                  dest='tagName', default='data_reprocessing_prompt',
                  help='Conditions-database global-tag name [data_reprocessing_prompt]')
(options, args) = parser.parse_args()

tagName = options.tagName

modes = {"coarse-start": 0, "coarse-width": 1, "fine-start": 2, "fine-width": 3}
if options.mode.isdecimal():
    mode = int(options.mode)
else:
    mode = modes[options.mode] if options.mode in modes else -1
if (mode < 0) or (mode >= 4):
    print("Illegal lookback-window mode {0}; use one of {1}".format(options.mode, modes))
    sys.exit()

if options.window is None:
    print("Required lookback-window values (-w min max step) not present")
    sys.exit()
if (options.window[0] < 0) or (options.window[1] < options.window[0]) or (options.window[2] <= 0):
    if options.window[0] < 0:
        print("Lookback-window min value {0} cannot be negative".format(options.window[0]))
    if options.window[1] < options.window[0]:
        print("Lookback-window max value {1} cannot be less than min value {0}".format(options.window[0], options.window[1]))
    if options.window[2] <= 0:
        print("Lookback-window step value {0} must be positive definite".format(options.window[2]))
    sys.exit()
window = options.window

inputName = re.sub(r"HLT.\.f0....", "HLT*.f*", options.infilename)
fileList = glob.glob(inputName)
if len(fileList) == 0:
    print("No file(s) match {0}".format(inputName))
    sys.exit()
print(fileList)
if not options.eNumber.isdecimal():
    print("Experiment number ({0}) is not valid".format(options.eNumber))
    sys.exit()
exp = '{0:04d}'.format(int(options.eNumber))
if not options.rNumber.isdecimal():
    print("Run number ({0}) is not valid".format(options.rNumber))
    sys.exit()
run = '{0:05d}'.format(int(options.rNumber))

histName = 'bklmHists-e{0}r{1}.root'.format(exp, run)
pdfName = 'bklmPlots-e{0}r{1}.pdf'.format(exp, run)

print('bklm-windowstart: exp=' + exp + ' run=' + run + ' input=' + inputName + '. Analyze all events using ' + tagName)

basf2.conditions.prepend_globaltag(tagName)

main = basf2.create_path()
if inputName.find(".sroot") >= 0:
    main.add_module('SeqRootInput', inputFileNames=fileList)
else:
    main.add_module('RootInput', inputFileName=inputName)
main.add_module('ProgressBar')

eventInspector = EventInspectorLookback(exp, run, histName, pdfName, mode, window)
rawdata.add_unpackers(main, components=['KLM'])
main.add_module('KLMReconstructor')
main.add_module(eventInspector)

basf2.process(main)
print(basf2.statistics)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Purpose:
#   Analyze DST [S]ROOT files with a sequence of RPC coarse lookback-window start values and write
#   resulting histograms/scatterplots to ROOT and PDF files.
#
# Prerequisite (on kekcc):
#   Before running this script, type
#     source /cvmfs/belle.cern.ch/tools/b2setup release-03-01-00 <or higher release>
#   then verify that the corresponding proper global tag is used near the end of this script.
#   (Global tags are tabulated at https://confluence.desy.de/display/BI/Global+Tag+%28GT%29+page)
#   The scripts bklmDB.py and EventInspectorWindowStart.py must be in the same folder as this script.
#
# Usage:
#   basf2 bklm-coarsewindowstart.py -- -e # -r # -i infilenames -t tagname
#      You need the '--' before these options to tell basf2 that these are options to this script.
#   Required arguments:
#      -i infilename  to specify the full pathname of the input [S]ROOT files, including wildcards (no default)
#      -e #   to specify the experiment number (no default)
#      -r #   to specify the run number (no default)
# Optional arguments:
#      -v #  to specify the "verbosity" of histogram plots (0=minimal, 1=all)
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
from basf2 import *
import sys
import re
import EventInspectorCoarseWindowStart
from EventInspectorCoarseWindowStart import *
import rawdata
from optparse import Option, OptionValueError, OptionParser
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
parser.add_option('-t', '--tagName',
                  dest='tagName', default='data_reprocessing_prompt',
                  help='Conditions-database global-tag name [data_reprocessing_prompt]')
parser.add_option('-v', '--verbosity',
                  dest='verbosity', default='0',
                  help='Histogram-plot verbosity [0=minimal, 1=all]')
(options, args) = parser.parse_args()

tagName = options.tagName

verbosity = int(options.verbosity)

inputName = re.sub("HLT.\.f0....", "HLT*.f*", options.infilename)
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

reset_database()
use_database_chain()
use_central_database(tagName)

main = create_path()
if inputName.find(".sroot") >= 0:
    main.add_module('SeqRootInput', inputFileNames=fileList)
else:
    main.add_module('RootInput', inputFileName=inputName)
main.add_module('ProgressBar')

eventInspector = EventInspectorCoarseWindowStart(exp, run, histName, pdfName, verbosity)
rawdata.add_unpackers(main, components=['BKLM'])
main.add_module('BKLMReconstructor')
main.add_module(eventInspector)

process(main)
print(statistics)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Purpose:
#   Analyze a DST ROOT file or an SROOT file and write resulting histograms/scatterplots to
#   ROOT and PDF files. This script cannot analyze MDST files because they don't contain RawKLMs.
#
# Prerequisite (on kekcc):
#   Before running this script, type
#     source /cvmfs/belle.cern.ch/tools/b2setup release-02-01-00 <or higher release>
#   then verify that the corresponding proper global tag is used near the end of this script.
#   (Global tags are tabulated at https://confluence.desy.de/display/BI/Global+Tag+%28GT%29+page)
#   The external python script bklmDB.py must be in the same folder as this script.
#
# Usage:
#   basf2 bklm-dst.py -- -e # -r # -i infilename -n # -d # -m # -t tagname
#      You need the '--' before these options to tell basf2 that these are options to this script.
#   Required arguments:
#      either -i infilename or -e # -r # (can supply all three)
#      -i infilename  to specify the full pathname of the input ROOT DST file (no default)
#      -e #   to specify the experiment number (no default)
#      -r #   to specify the run number (no default)
# Optional arguments:
#      -n #   to specify the maximum number of events to analyze (no default -> all events)
#      -d #   to specify the maximum number of event displays (default is 100)
#      -m #   to specify the minimum number of RPC BKLMHit2ds in any one sector (default is 4)
#      -t tagName   to specify the name of conditions-database global tag (no default)
#      -l #   to specify whether to use legacy time calculations (1) or not (0) (default is 0)
#
# Input:
#   ROOT DST file written by basf2 (may include multiple folios for one expt/run). For example,
#   /ghi/fs01/belle2/bdata/Data/Raw/e0003/r04794/sub00/physics.0003.r04794.HLT2.f*.root
#   /ghi/fs01/belle2/bdata/Data/Raw/e0004/r06380/sub00/cosmic.0004.r06380.HLT2.f00000.root
#
# Output:
#   ROOT histogram file named bklmHists-e#r#.root, using the experiment number and run number
#   PDF file named bklmHists-e#r#.pdf, using the experiment number and run number
#

import basf2
from basf2 import *
import EventCountLimiter
from EventCountLimiter import *
import EventInspector
from EventInspector import *
import simulation
import reconstruction
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
parser.add_option('-n', '--nEvents',
                  dest='nEvents', default='',
                  help='Max # of analyzed events [no default]')
parser.add_option('-d', '--displays',
                  dest='displays', default='100',
                  help='Max # of displayed events [100]')
parser.add_option('-m', '--minRPCHits',
                  dest='minRPCHits', default='4',
                  help='Min # of RPC hits in any one sector to display the event [4]')
parser.add_option('-l', '--legacyTimes',
                  dest='legacyTimes', default='0',
                  help='Perform legacy time calculations (1) or not (0) for BKLMHit1ds,2ds [0]')
parser.add_option('-t', '--tagName',
                  dest='tagName', default='data_reprocessing_prompt',
                  help='Conditions-database global-tag name [data_reprocessing_prompt]')
(options, args) = parser.parse_args()

maxCount = -1
if options.nEvents != '':
    maxCount = int(options.nEvents)
    if maxCount <= 0:
        print("Maximum number of events to analyze is", maxCount, " - nothing to do.")
        sys.exit()

maxDisplays = int(options.displays)

minRPCHits = int(options.minRPCHits)

legacyTimes = int(options.legacyTimes)

tagName = options.tagName

inputName = ''
exp = ''
run = ''
if options.infilename != '':
    inputName = options.infilename
    fileList = glob.glob(inputName)
    if len(fileList) == 0:
        print("No file(s) match {0}".format(inputName))
        sys.exit()
if options.eNumber != '':
    if not options.eNumber.isdecimal():
        print("Experiment number ({0}) is not valid".format(options.eNumber))
        sys.exit()
    exp = '{0:04d}'.format(int(options.eNumber))
else:
    eStart = inputName.find('/e') + 2
    if eStart < 0:
        print("Input filename does not contain the required experiment number")
        sys.exit()
    eEnd = inputName.find('/', eStart)
    exp = inputName[eStart:eEnd]
    if not exp.isdecimal():
        print("Input filename's experiment number ({0}) is not valid".format(exp))
        sys.exit()
if options.rNumber != '':
    if not options.rNumber.isdecimal():
        print("Run number ({0}) is not valid".format(options.rNumber))
        sys.exit()
    run = '{0:05d}'.format(int(options.rNumber))
else:
    rStart = inputName.find('/r') + 2
    if rStart < 0:
        print("Input filename does not contain the required run number")
        sys.exit()
    rEnd = inputName.find('/', rStart)
    run = inputName[rStart:rEnd]
    if not run.isdecimal():
        print("Input filename's run number ({0}) is not valid".format(run))
        sys.exit()
if len(inputName) == 0:
    fileList = glob.glob('/ghi/fs01/belle2/bdata/Data/Raw/e{0}/r{1}/sub00/*.{0}.{1}.HLT2.f00000.root'.format(exp, run))
    if len(fileList) == 0:
        print("No file(s) found for experiment <{0}> run <{1}>".format(options.eNumber, options.rNumber))
        sys.exit()
    inputName = fileList[0].replace("f00000", "f*")

histName = 'bklmHists-e{0}r{1}.root'.format(exp, run)
pdfName = 'bklmPlots-e{0}r{1}.pdf'.format(exp, run)
eventPdfName = 'bklmEvents-e{0}r{1}.pdf'.format(exp, run)

if maxCount >= 0:
    print('bklm-dst: exp=' + exp + ' run=' + run + ' input=' + inputName + '. Analyze', maxCount, 'events using ' + tagName)
else:
    print('bklm-dst: exp=' + exp + ' run=' + run + ' input=' + inputName + '. Analyze all events using ' + tagName)

reset_database()
use_database_chain()
use_central_database(tagName)

main = create_path()
if inputName.find(".sroot") >= 0:
    main.add_module('SeqRootInput', inputFileNames=inputName)
else:
    main.add_module('RootInput', inputFileName=inputName)
main.add_module('ProgressBar')

eventInspector = EventInspector(exp, run, histName, pdfName, eventPdfName, maxDisplays, minRPCHits, legacyTimes)
if maxCount >= 0:
    child = create_path()
    eventCountLimiter = EventCountLimiter(maxCount)
    eventCountLimiter.if_true(child, AfterConditionPath.CONTINUE)
    main.add_module(eventCountLimiter)
    rawdata.add_unpackers(child, components=['BKLM'])
    child.add_module('BKLMReconstructor')
    child.add_module(eventInspector)
else:
    rawdata.add_unpackers(main, components=['BKLM'])
    main.add_module('BKLMReconstructor')
    main.add_module(eventInspector)

process(main)
print(statistics)

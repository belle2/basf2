#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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
#      -s #   to select events with all (0) or exactly one (1) or two or more (2) entries/channel (default is 0)
#      -n #   to specify the maximum number of events to analyze (no default -> all events)
#      --verbosity #   to specify how many histograms to save in the PDF file (0=minimal, 1=all) [default is 0]
#      -d #   to specify the maximum number of event displays (default is 0)
#      -m #   to specify the minimum number of RPC BKLMHit2ds in any one sector (default is 4)
#      -t tagName   to specify the name of conditions-database global tag (no default)
#      -l #   to specify whether to use legacy time calculations (1) or not (0) (default is 0)
#
# Input:
#   ROOT DST file written by basf2 (may include multiple folios for one expt/run). For example,
#   /ghi/fs01/belle2/bdata/Data/Raw/e0003/r04794/sub00/physics.0003.r04794.HLT1.f*.root
#   /ghi/fs01/belle2/bdata/Data/Raw/e0004/r06380/sub00/cosmic.0004.r06380.HLT1.f00000.root
#   /ghi/fs01/belle2/bdata/Data/Raw/e0007/r01650/sub00/cosmic.0007.r01650.HLT1.f*.root
#
# Output:
#   ROOT histogram file named bklmHists-e#r#.root, using the experiment number and run number
#   PDF file named bklmHists-e#r#.pdf, using the experiment number and run number
#

import basf2
import sys
import re
import EventInspector
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
parser.add_option('-n', '--nEvents',
                  dest='nEvents', default='',
                  help='Max # of analyzed events [no default]')
parser.add_option('-s', '--singleEntry', type="int",
                  dest='singleEntry', default=0,
                  help='Select events with any (0) or exactly one (1) or more than one (2) entries/channel [0]')
parser.add_option('--verbosity', type="int",
                  dest='verbosity', default=0,
                  help='How many histograms to save (0=minimal, 1=all) [0]')
parser.add_option('-d', '--displays', type="int",
                  dest='displays', default=0,
                  help='Max # of displayed events [0]')
parser.add_option('-v', '--view', type="int",
                  dest='view', default=2,
                  help='View event displays using one-dimensional (1) or two-dimensional (2) hits [2]')
parser.add_option('-m', '--minRPCHits', type="int",
                  dest='minRPCHits', default=4,
                  help='Min # of RPC hits in any one sector to display the event [4]')
parser.add_option('-l', '--legacyTimes', type="int",
                  dest='legacyTimes', default=0,
                  help='Perform legacy time calculations (1) or not (0) for BKLMHit1ds,2ds [0]')
parser.add_option('-t', '--tagName',
                  dest='tagName', default='data_reprocessing_prompt',
                  help='Conditions-database global-tag name [data_reprocessing_prompt]')
(options, args) = parser.parse_args()

singleEntry = options.singleEntry
if singleEntry < 0 or singleEntry > 2:
    singleEntry = 0

maxCount = -1
if options.nEvents != '':
    maxCount = int(options.nEvents)
    if maxCount <= 0:
        print("Maximum number of events to analyze is", maxCount, " - nothing to do.")
        sys.exit()

verbosity = options.verbosity

view = options.view

maxDisplays = options.displays

minRPCHits = options.minRPCHits

legacyTimes = options.legacyTimes

tagName = options.tagName

inputName = ''
exp = ''
run = ''
if options.infilename != '':
    inputName = re.sub(r"HLT.\.f0....", "HLT*.f*", options.infilename)
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
    inputName = '/ghi/fs01/belle2/bdata/Data/Raw/e{0}/r{1}/sub00/*.{0}.{1}.HLT*.f*.root'.format(exp, run)
    fileList = glob.glob(inputName)
    if len(fileList) == 0:
        print("No file(s) found for experiment <{0}> run <{1}>".format(options.eNumber, options.rNumber))
        sys.exit()

suffix = '' if singleEntry == 0 else '-singleEntry' if singleEntry == 1 else '-multipleEntries'
histName = 'bklmHists-e{0}r{1}{2}.root'.format(exp, run, suffix)
pdfName = 'bklmPlots-e{0}r{1}{2}.pdf'.format(exp, run, suffix)
eventPdfName = 'bklmEvents{3}D-e{0}r{1}{2}.pdf'.format(exp, run, suffix, view)

if maxCount >= 0:
    print('bklm-dst: exp=' + exp + ' run=' + run + ' input=' + inputName + '. Analyze', maxCount, 'events using ' + tagName)
else:
    print('bklm-dst: exp=' + exp + ' run=' + run + ' input=' + inputName + '. Analyze all events using ' + tagName)

basf2.conditions.prepend_globaltag(tagName)

main = basf2.create_path()
if inputName.find(".sroot") >= 0:
    main.add_module('SeqRootInput', inputFileNames=inputName)
else:
    main.add_module('RootInput', inputFileName=inputName)
main.add_module('ProgressBar')

eventInspector = EventInspector(exp, run, histName, pdfName, eventPdfName, verbosity,
                                maxDisplays, minRPCHits, legacyTimes, singleEntry, view)
rawdata.add_unpackers(main, components=['KLM'])
main.add_module('KLMReconstructor')
main.add_module(eventInspector)

basf2.process(main, max_event=maxCount)
print(basf2.statistics)

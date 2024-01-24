#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Purpose:
#   Analyze a DST file and write a PDF of "interesting" event displays that includes
#   BKLMHit2ds, ExtHits, and MuidHits.
#   This script cannot analyze MDST files because they don't contain RawKLMs, BKLMHit2ds, nor MuidHits.
#
# Prerequisites (on kekcc):
#   Before running this script, type
#     source /cvmfs/belle.cern.ch/tools/b2setup release-02-01-00 <or higher release>
#   then verify that the corresponding proper global tag is used near the end of this script.
#   (Global tags are tabulated at https://confluence.desy.de/display/BI/Global+Tag+%28GT%29+page)
#   The external python script bklmDB.py must be in the same folder as this script.
#
# Usage:
#   basf2 bklm-eventdisplay.py -- -e # -r # -i infilename -n # -d # -m # -u # -t tagname
#      You need the '--' before these options to tell basf2 that these are options to this script.
#   Required arguments:
#      either -i infilename or -e # -r # (can supply all three)
#      -i infilename  to specify the full pathname of the input ROOT DST file (no default)
#      -e #   to specify the experiment number, e.g., -e 1 (no default)
#      -r #   to specify the run number, e.g., -r 4794 (no default)
#   Optional arguments:
#      -n #   to specify the maximum number of events to analyze (no default -> all events)
#      -d #   to specify the maximum number of displayed events to write to the PDF file (default = 100)
#      -m #   to specify the minimum number of RPC hits in one sector (default = 4)
#      -u #   to specify the minimum number of Muid hits in the event (default = 1)
#      -t tagname   to specify the name of conditions database global tag (no default)
#
# Input:
#   ROOT DST file written by basf2 (may include multiple folios for one expt/run). For example,
#   /ghi/fs01/belle2/bdata/Data/Raw/e0003/r04794/sub00/physics.0003.r04794.HLT2.f*.root
#   /ghi/fs01/belle2/bdata/Data/Raw/e0004/r06380/sub00/cosmic.0004.r06380.HLT2.f00000.root
#
# Output:
#   PDF file named bklmEvents-e#r#.pdf, using the experiment number and run number
#

import basf2
import EventDisplayer
import sys
from tracking import add_tracking_reconstruction
import rawdata
from optparse import OptionParser
import glob

parser = OptionParser()
parser.add_option('-i', '--inputfile',
                  dest='infilename', default='',
                  help='Input ROOT filename [no default]')
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
                  help='Max # of displayed events [default=100]')
parser.add_option('-m', '--minRPCHits',
                  dest='minRPCHits', default='4',
                  help='Minimum # of RPC hits in one sector [default=4]')
parser.add_option('-u', '--muids',
                  dest='minMuidHits', default='1',
                  help='Minimum # of Muid hits in the event [default=1]')
parser.add_option('-t', '--tag',
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

minMuidHits = int(options.minMuidHits)

tagName = options.tagName

inputName = ''
exp = ''
run = ''
if options.infilename != '':
    inputName = options.infilename
    fileList = glob.glob(inputName)
    if len(fileList) == 0:
        print(f'No file(s) match {inputName}')
        sys.exit()
if options.eNumber != '':
    if not options.eNumber.isdecimal():
        print(f'Experiment number ({options.eNumber}) is not valid')
        sys.exit()
    exp = f'{int(options.eNumber):04d}'
else:
    eStart = inputName.find('/e') + 2
    if eStart < 0:
        print('Input filename does not contain the required experiment number')
        sys.exit()
    eEnd = inputName.find('/', eStart)
    exp = inputName[eStart:eEnd]
    if not exp.isdecimal():
        print(f'Input filename experiment number({exp}) is not valid')
        sys.exit()
if options.rNumber != '':
    if not options.rNumber.isdecimal():
        print(f'Run number ({options.rNumber}) is not valid')
        sys.exit()
    run = f'{int(options.rNumber):05d}'
else:
    rStart = inputName.find('/r') + 2
    if rStart < 0:
        print('Input filename does not contain the required run number')
        sys.exit()
    rEnd = inputName.find('/', rStart)
    run = inputName[rStart:rEnd]
    if not run.isdecimal():
        print(f'Input filename run number({run}) is not valid')
        sys.exit()
if len(inputName) == 0:
    fileList = glob.glob(f'/ghi/fs01/belle2/bdata/Data/Raw/e{exp}/r{run}/sub00/*.{exp}.{run}.HLT2.f00000.root')
    if len(fileList) == 0:
        print(f'No file(s) found for experiment <{options.eNumber}> run <{options.rNumber}>')
        sys.exit()
    inputName = fileList[0].replace('f00000', 'f*')

eventPdfName = f'bklmEvents-e{exp}r{run}.pdf'

if maxCount >= 0:
    print('bklm-display: exp=' + exp + ' run=' + run + ' input=' + inputName + '. Analyze', maxCount, 'events using ' + tagName)
    print('   Write at most', maxDisplays, 'event displays, requiring # RPC hits per sector >=', minRPCHits,
          '  # Muids in event >=', minMuidHits)
else:
    print('bklm-display: exp=' + exp + ' run=' + run + ' input=' + inputName + '. Analyze all events using ' + tagName)
    print('   Write at most', maxDisplays, 'event displays, requiring # RPC hits per sector >=', minRPCHits,
          '  # Muids in event >=', minMuidHits)

basf2.conditions.prepend_globaltag(tagName)

main = basf2.create_path()
main.add_module('RootInput', inputFileName=inputName)
main.add_module('ProgressBar')

eventDisplayer = EventDisplayer(exp, run, eventPdfName, maxDisplays, minRPCHits, minMuidHits)
rawdata.add_unpackers(main)
main.add_module('KLMReconstructor')
add_tracking_reconstruction(main)
ext = main.add_module('Ext')
ext.param('pdgCodes', [13])
muid = main.add_module('Muid')
# muid.param('MaxDistSigma', 10.0)
main.add_module(eventDisplayer)

basf2.process(main, max_event=maxCount)
print(basf2.statistics)

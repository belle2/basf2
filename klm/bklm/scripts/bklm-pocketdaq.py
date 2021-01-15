#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Purpose:
#   Analyze PocketDAQ data created by the test stand at Indiana University
#   and write a PDF file of the resulting histograms/scatterplots.
#
# Prerequisites (on kekcc):
#   Before running this script, type
#     source /cvmfs/belle.cern.ch/tools/b2setup release-02-01-00 <or higher release>
#   and translate the test-stand raw data file to a ROOT file using Richard Peschke's app
#
# Usage:
#   basf2 bklm-pocketdaq.py -- -i infilename -e # -r # -n # -f #
#   You need the '--' before these options to tell basf2 that these are options to this script.
#   Required argument:
#      -i infilename  to specify the full pathname of the input ROOT file
#   Optional arguments:
#      -e #   to specify the experiment number, e.g., -e 1 (no default)
#      -r #   to specify the run number, e.g., -r 0109 (no default)
#      -n #   to specify the number of events in the run to be processed (no default -> all events)
#      -f #   to filter only the even-numbered events (-f 0) or the odd-numbered events (-f 1) (no default -> all events)
#
# Input:
#   ROOT file written by Richard Peschke's translator app whose input was a test-stand raw data file. For example,
#   /home/belle2/dbiswas/Documents/PocketDAQ/klm_offline_entpacker/build/klm_unpacker/brandon_data/dblpls_30k_fine_20190109.root
#
# Output:
#   ROOT histogram file named bklmHists-e#r#.root, using the experiment number and run number
#   PDF file named bklmPlots-e#r#.pdf, using the experiment number and run number
#

import EventInspectorPocketDAQ
from ROOT import TFile
import sys
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
                  help='# of events to process [no default]')
parser.add_option('-f', '--filter',
                  dest='eventFilter', default='',
                  help='Event filter (0=evens, 1=odds) [no default]')
(options, args) = parser.parse_args()

maxCount = -1
if options.nEvents != '':
    maxCount = int(options.nEvents)
    if maxCount <= 0:
        print("Maximum number of events to analyze is", maxCount, " - nothing to do.")
        sys.exit()

eventFilter = -1
if (options.eventFilter == '0') or (options.eventFilter == '1'):
    eventFilter = int(options.eventFilter)

inputName = ''
exp = ''
run = ''
if options.infilename != '':
    inputName = options.infilename
    fileList = glob.glob(inputName)
    if len(fileList) == 0:
        print("No file(s) match {0}".format(inputName))
        sys.exit()
    inputName = fileList[0].replace("f00000", "f*")
else:
    print("Missing input filename (required parameter) for experiment <{0}> run <{1}>".format(options.eNumber, options.rNumber))
    sys.exit()
if not options.eNumber.isdecimal():
    print("Experiment number ({0}) is not valid".format(options.eNumber))
    sys.exit()
exp = '{0:04d}'.format(int(options.eNumber))
if not options.rNumber.isdecimal():
    print("Run number ({0}) is not valid".format(options.rNumber))
    sys.exit()
run = '{0:05d}'.format(int(options.rNumber))

infile = TFile(fileList[0])
histName = 'bklmHists-e{0}r{1}.root'.format(exp, run)
pdfName = 'bklmPlots-e{0}r{1}.pdf'.format(exp, run)

if eventFilter == 0:
    print('bklm-pocketdaq: exp=' + exp + ' run=' + run + ' input=', options.infilename + ' - processing even-numbered events')
elif eventFilter == 1:
    print('bklm-pocketdaq: exp=' + exp + ' run=' + run + ' input=', options.infilename + ' - processing odd-numbered events')
else:
    print('bklm-pocketdaq: exp=' + exp + ' run=' + run + ' input=', options.infilename)

inspector = EventInspectorPocketDAQ(exp, run, histName, pdfName)
inspector.initialize()
inspector.beginRun()
count = 0
eventNumber = -1
eventHits = []
tt_ctime = 0
raw_time = 0
for row in infile.Get('KLM_raw_hits'):
    # (optional) stop processing after maxCount
    if (maxCount > 0) and (row.eventNr >= maxCount):
        break
    # (optional) process only the filtered events
    if (eventFilter >= 0) and (row.eventNr % 2) != eventFilter:
        continue
    items = (row.lane, row.channel, row.axis, row.ctime, row.tdc, row.charge)
    newEventNumber = row.eventNr
    if newEventNumber == eventNumber:
        eventHits.append(items)
    else:
        if eventNumber >= 0:
            inspector.event(eventHits, tt_ctime, raw_time)
        count = count + 1
        eventNumber = newEventNumber
        if row.broken != 0:
            print('*** Event {0} is broken!'.format(eventNumber))
        eventHits.clear()
        eventHits.append(items)
        tt_ctime = row.tt_ctime << 3
        raw_time = (row.raw_time >> 16) << 3
if eventNumber >= 0:
    inspector.event(eventHits, tt_ctime, raw_time)
inspector.endRun()
inspector.terminate()
print('# of analyzed events = ', count)

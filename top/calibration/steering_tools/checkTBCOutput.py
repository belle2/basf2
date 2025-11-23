#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------------
# Check TBC output root files for the deeps in sample occupancies.
#
# usage: basf2 checkTBCOutput.py <path_to_files>
#        <path_to_files> can include wildcards (prepended by backslash), example: tbc/\*/\*.root
# ---------------------------------------------------------------------------------------------

import sys
import glob
from ROOT import TFile
from basf2.utils import pretty_print_table


if len(sys.argv) < 2:
    print('usage: basf2', sys.argv[0], '<path_to_files>')
    print('       <path_to_files> can include wildcards (prepended by backslash), example: tbc/\\*/\\*.root')
    sys.exit()

files = glob.glob(sys.argv[1])
if len(files) == 0:
    print('No root files found in ' + sys.argv[1])
    sys.exit()


def num_below_cut(h):
    '''
    Returns the number of samples with low occupancy.
    :param h: histogram of occupancy (number of calpulses per sample)
    '''

    h.Rebin(2)
    cut = h.Integral() / h.GetNbinsX() * 0.8
    s = 0
    n = 0
    for i in range(h.GetNbinsX()):
        y = h.GetBinContent(i + 1)
        if y > cut:
            s += y
            n += 1
    cut = s / n * 0.7
    n = 0
    for i in range(h.GetNbinsX()):
        y = h.GetBinContent(i + 1)
        if y < cut:
            n += 1
    return n


def print_table(title, table):
    '''
    Prints the table.
    :param title: table title
    :param table: table
    '''

    print()
    print(title)
    table_rows = [['bs' + str(b) + ' '] + [str(table[s][b]) for s in range(16)] for b in range(4)]
    table_rows.insert(0, [''] + ['s' + str(s) for s in range(1, 17)])
    pretty_print_table(table_rows, [4 for i in range(17)])


ncal = 0
nlow = 0
numcal = [[0 for bs in range(4)] for slot in range(16)]
numlow = [[0 for bs in range(4)] for slot in range(16)]
for fileName in sorted(files):
    try:
        slot_bs = fileName.split('/')[-1].split('-')[0].split('tbcSlot')[1].split('_')
        slot = int(slot_bs[0])
        bs = int(slot_bs[1])
        print(fileName, 'slot =', slot, 'bs =', bs)
    except BaseException:
        print(fileName, '--> file ignored')
        continue
    tfile = TFile.Open(fileName)
    if not tfile:
        print('--> Error: cannot open this file')
        continue
    success = tfile.Get("success")
    if not success:
        print('--> Error: histogram named success not found')
        continue
    nc = 0
    for chan in range(success.GetNbinsX()):
        if success.GetBinContent(chan + 1) == 0:
            continue
        ncal += 1
        nc += 1
        numcal[slot-1][bs] += 1
        hname = 'sampleOccup_ch' + str(chan)
        h = tfile.Get(hname)
        if not h:
            print('--> Error: sample occupancy histogram for channel ' + str(chan) + ' not found')
            continue
        n = num_below_cut(h)
        if n > 2:
            print('  ' + h.GetTitle(), 'is low in', 2 * n, '/ 256 samples')
            nlow += 1
            numlow[slot-1][bs] += 1
    print('-->', nc, 'channels calibrated')

print_table('Number of successfully calibrated channels:', numcal)
print_table('Number of channels with deeps in sample occupancy:', numlow)

print('\nCalibration summary: ')
print(' - successfully calibrated channels:', ncal, '/', 512 * 16)
print(' - constants maybe not reliable for', nlow, '/', ncal, 'channels.',
      'If this fraction is large, check calpulse selection window in runTBC.py.')

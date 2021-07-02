#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


def fFileExist(filename):
    '''Test if file exists'''

    try:
        oFile = open(filename)
    except OSError:
        return 0
    else:
        oFile.close()
        return 1


def read(optlevel):
    '''read time from output files'''

    filename = 'output/output-' + optlevel + '.dat'
    if fFileExist(filename) == 0:
        print('missing ' + filename)
    else:
        CDCLegendreTracking = []
        fobj = open(filename)
        # read output file
        for line in fobj:
            if line.startswith('CDCLegendreTracking'):
                words = line.split()
                CDCLegendreTracking.append(float(words[6]))
        fobj.close()
        print(optlevel + ' read')
        # write CDCLegendreTracking times
        fobj = open('out/' + optlevel + '.out', 'w')
        for i in range(0, len(CDCLegendreTracking)):
            fobj.write(str(CDCLegendreTracking[i]) + '\n')
            i = i + 1
        fobj.close()


optlevel = ['gcc-O0', 'gcc-O3', 'gcc-O3-native']
for i in optlevel:
    read(i)

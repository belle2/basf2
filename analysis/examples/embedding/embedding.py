#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import subprocess

if __name__ == "__main__":

    # particleList name of the skim
    removeList = 'B+:BplusToKplusJpsiMuMu'
    signalList = 'B+:BplusToKplusNuNu'

    # example files that contain the particleList
    inputDataForRemoveList = b2.find_file('embedding_skim_BplusToKplusJpsiMuMu_bucket17.udst.root', 'examples', False)
    inputMCForSignalList = b2.find_file('embedding_skim_BplusToKplusJpsiMuMu_bucket17.udst.root', 'examples', False)

    outputDataForRemoveList = 'data.root'
    outputMCForSignalList = 'sigmc.root'
    outputEmbedded = 'embedded_data.root'

    subprocess.call('basf2 udst_purge_list.py -- ' +
                    ' --listName ' + removeList +
                    ' --charge pos ' +  # pos: B+, neg:B-, zero:B0 (B0bar)
                    ' --fileIn ' + inputDataForRemoveList +
                    ' --fileOut ' + outputDataForRemoveList,
                    shell=True)

    subprocess.call('basf2 udst_purge_list.py -- ' +
                    ' --isSignal ' +  # for signal MC
                    ' --listName ' + signalList +
                    ' --charge pos ' +  # pos: B+, neg:B-, zero:B0 (B0bar)
                    ' --fileIn ' + inputMCForSignalList +
                    ' --fileOut ' + outputMCForSignalList,
                    shell=True)

    subprocess.call('basf2 embedding_merge.py --' +
                    ' --filePrimary ' + outputDataForRemoveList +
                    ' --fileSecondary ' + outputMCForSignalList +
                    ' --fileOut ' + outputEmbedded,
                    shel=True)

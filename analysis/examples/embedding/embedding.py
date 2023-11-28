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
    inputMCForSignalList = b2.find_file('embedding_skim_sig_71_Bplus2Kplus.udst.root', 'examples', False)

    outputDataForRemoveList = 'data.root'
    outputMCForSignalList = 'sigmc.root'
    outputEmbedded = 'embedded_data.root'

    cmd = f'basf2 udst_purge_list.py -- --listName {removeList} --charge pos --fileIn {inputDataForRemoveList}'\
          f' --fileOut {outputDataForRemoveList}'
    subprocess.check_call(cmd.split())

    cmd = f'basf2 udst_purge_list.py -- --isSignal --listName {signalList} --charge pos --fileIn {inputMCForSignalList}'\
          f' --fileOut {outputMCForSignalList}'
    subprocess.check_call(cmd.split())

    cmd = f'basf2 embedding_merge.py -- --filePrimary {outputDataForRemoveList} --fileSecondary {outputMCForSignalList}'\
          f' --fileOut {outputEmbedded}'
    subprocess.check_call(cmd.split())

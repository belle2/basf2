#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# --------------------------------------------------------------------------
# Prints Calibration trigger results from a raw sroot file
# --------------------------------------------------------------------------

import basf2 as b2
from ROOT import Belle2

from collections import defaultdict

results = defaultdict(int)


class PrintCalibTriggerResults(b2.Module):

    '''
    Prints Calibration trigger results in a well formatted way.
    User is prompted to continue or quit at each event
    '''

    def event(self):
        '''
        Print log likelihoods and wait for user respond.
        '''

        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        exp = evtMetaData.obj().getExperiment()
        run = evtMetaData.obj().getRun()
        evt = evtMetaData.obj().getEvent()
        print()
        print('Experiment ' + str(exp) + ' Run ' + str(run) + ' Event ' + str(evt) + ':')
        print()
        trigger_result = Belle2.PyStoreObj('SoftwareTriggerResult')
        for name, result in trigger_result.getResults():
            print('Result ' + str(name) + ': ' + str(result))
            if result == 1:
                global results
                results[name] += 1
        print('')


b2.set_log_level(b2.LogLevel.ERROR)

# Create path
main = b2.create_path()

# Root input
roinput = b2.register_module('SeqRootInput')
main.add_module(roinput)

# print array of log likelihoods
main.add_module(PrintCalibTriggerResults())

# Process events
b2.process(main)

print(results)

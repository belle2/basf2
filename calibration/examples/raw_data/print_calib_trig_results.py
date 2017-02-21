#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------
# Prints Calibration trigger results from a raw sroot file
# --------------------------------------------------------------------------

from basf2 import *
from ROOT import Belle2

from collections import defaultdict

results = defaultdict(int)


class PrintCalibTriggerResults(Module):

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


set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# Root input
roinput = register_module('SeqRootInput')
main.add_module(roinput)

# print array of log likelihoods
main.add_module(PrintCalibTriggerResults())

# Process events
process(main)

print(results)

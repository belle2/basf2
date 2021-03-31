#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------
# Prints PIDLikelihoods from a mdst file
# File name must be specified using -i switch:
#   basf2 mdst/examples/printPIDLikelihoods.py -i <fileName.root>
# --------------------------------------------------------------------------

import basf2 as b2
from ROOT import Belle2


class printPIDLikelihoods(b2.Module):

    '''
    Prints PID log likelihoods + basic track info in a well formatted way.
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
        tracks = Belle2.PyStoreArray('Tracks')
        for track in tracks:
            fitresult = track.getTrackFitResult(Belle2.Const.pion)
            if fitresult:
                p_mag = fitresult.getMomentum().Mag()
                pmom = '{:5.3f}'.format(p_mag) + ' GeV/c'
            else:
                pmom = '?'
            mcpart = track.getRelated('MCParticles')
            if mcpart:
                pdg = str(mcpart.getPDG())
            else:
                pdg = '?'
            index = track.getArrayIndex()
            print('Track ' + str(index) + ': p = ' + pmom + '  MCtruth = ' + pdg)
            likelihood = track.getRelated('PIDLikelihoods')
            try:
                print('logLikelihoods:')
                likelihood.printArray()
            except BaseException:
                print('--> No relation to PIDLikelihood')
            print()

        # wait for user respond
        response = input("Type Enter to continue or Q to quit.\n").lower().strip()
        if response == "q":
            evtMetaData.obj().setEndOfData()


b2.set_log_level(b2.LogLevel.ERROR)

# Create path
main = b2.create_path()

# Root input
roinput = b2.register_module('RootInput')
main.add_module(roinput)

# print array of log likelihoods
main.add_module(printPIDLikelihoods())

# Process events
b2.process(main)

#!/usr/bin/env python
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------
# Prints PIDLikelihoods from a mdst file
# File name must be specified using -i switch:
#   basf2 mdst/examples/printPIDLikelihoods.py -i <fileName.root>
# --------------------------------------------------------------------------

from basf2 import *
from ROOT import Belle2


class printPIDLikelihoods(Module):

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
        print
        print 'Experiment ' + str(exp) + ' Run ' + str(run) + ' Event ' \
            + str(evt) + ':'
        print
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
            print 'Track ' + str(index) + ': p = ' + pmom + '  MCtruth = ' \
                + pdg
            likelihood = track.getRelated('PIDLikelihoods')
            try:
                print 'logLikelihoods:'
                likelihood.printArray()
            except:
                print '--> No relation to PIDLikelihood'
            print

        # wait for user respond
        try:
            q = 0
            Q = 0
            abc = input('Type <CR> to continue or Q to quit ')
            evtMetaData.obj().setEndOfData()
        except:
            abc = ''  # dummy line to terminate try-except


set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# Root input
roinput = register_module('RootInput')
main.add_module(roinput)

# print array of log likelihoods
main.add_module(printPIDLikelihoods())

# Process events
process(main)


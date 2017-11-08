#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import Module
from ROOT import Belle2


class EffModule(Module):
    """
    This module is to calculate and print out the efficiency of each L1 trigger line with
    the trigger result from object 'TRGGDLResults'
    """
    #: The total number of events
    Ntot_event = 0
    #: The number of events passing L1 trigger
    Ntrg_event = 0
    #: The number of events passing each L1 trigger line
    Nsubtrg_event = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    prescale = [1, 1, 20, 1, 1, 1, 1, 1, 1, 1, 1, 10, 10, 1, 1, 1, 1, 1]
#   trglog = ['n_2d_finder>=3', 'n_2d_finder==2&&BhabhaVeto==0',
    trglog = ['3 or more 3D tracks',
              '2 3D tracks, ≥1 within 25 cm, not a trkBhabha',
              '2 3D tracks, not a trkBhabha',
              '2 3D tracks, trkBhabha',
              '1 track, <25cm, clust same hemi, no 2 GeV clust',
              '1 track, <25cm, clust opp hemi, no 2 GeV clust',
              '≥3 clusters inc. ≥1 300 MeV, not an eclBhabha',
              '2 GeV E* in [4,14], not a trkBhabha',
              '2 GeV E* in [4,14], trkBhabha',
              '2 GeV E* in 2,3,15 or 16, not a trkBhabha or eclBhabha',
              '2 GeV E* in 2,3,15 or 16, trkBhabha or eclBhabha',
              '2 GeV E* in 1 or 17, not a trkBhabha or eclBhabha',
              '2 GeV E* in 1 or 17, trkBhabha or eclBhabha',
              'exactly 1 E*>1 GeV and 1 E>300 MeV, in [4,15]',
              'exactly 1 E*>1 GeV and 1 E>300 MeV, in 2,3 or 16',
              'clusters back-to-back in phi, both >250 MeV, no 2 GeV',
              'clusters back-to-back in phi, 1 <250 MeV, no 2 GeV',
              'clusters back-to-back in 3D, no 2 GeV'
              ]
    # ---add new trigger line by users---
    # ---add a component with initial value 0 in Nsubtrg_event
    # Nsubtrg_event+=[0]
    # ---add the prescale factor in prescale list
    # prescale += [1]
    # ---add the description of new trigger logics in trglog
    # trglog+=['new trg logics']

    def event(self):
        """
        Event function to count the numbers of events passing each trigger line
        """
        self.Ntot_event += 1
        trgresult = Belle2.PyStoreObj('TRGSummary')
        summary = trgresult.getTRGSummary(0)
        if summary >= 1:
            self.Ntrg_event += 1
        sum_bin = bin(summary)
        for i in range(len(sum_bin) - 2):
            trg = int(sum_bin[len(sum_bin) - 1 - i])
            if trg == 1:
                self.Nsubtrg_event[i] += 1

    def terminate(self):
        """
        Calculate the efficiency of each trigger line with the statistical values in event function
        """
        if self.Ntot_event == 0:
            return
        sp = ' '
        print('\n')
        eff_tot = self.Ntrg_event / self.Ntot_event * 100.0
        print('L1 Trigger efficiency(%%): %6.2f' % (eff_tot))
        print('Trigger Line', 5 * sp, 'PreScale Factor', 3 * sp, 'Efficiency(%)', 3 * sp, 'Logics')
        ntrg = len(self.Nsubtrg_event)
        if self.Ntot_event != 0:
            for i in range(ntrg):
                eff = self.Nsubtrg_event[i] / self.Ntot_event * 100.0
                print('T%3d                %4d              %6.2f              %s ' % (i, self.prescale[i], eff, self.trglog[i]))


def EffCalculation(path):
    path.add_module(EffModule())

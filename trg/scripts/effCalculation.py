#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *
from ROOT import *
from ROOT import Belle2


class EffModule(Module):
    Ntot_event = 0
    Ntrg_event = 0
    Nsubtrg_event = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

    def event(self):
        self.Ntot_event += 1
        trgresult = Belle2.PyStoreObj('TRGGDLResults')
        summary = trgresult.getL1TriggerResults()
        if summary >= 1:
            self.Ntrg_event += 1
        sum_bin = bin(summary)
        for i in range(len(sum_bin) - 2):
            trg = int(sum_bin[len(sum_bin) - 1 - i])
            if trg == 1:
                self.Nsubtrg_event[i] += 1

    def terminate(self):
        sp = ' '
        print('\n')
        eff_tot = self.Ntrg_event / self.Ntot_event
        print('L1 Trigger efficiency: %6.2f' % (eff_tot))
        print('Trigger Line', 5 * sp, 'PreScale Factor', 3 * sp, 'Efficiency', 3 * sp, 'Logics')
        ntrg = 12
        prescale = [1, 1, 1, 1, 1, 1, 2000, 2000, 2000, 1000, 1000, 1000]
        trglog = ['n_2d_finder>=3', 'n_2d_finder==2&&BhabhaVeto==0',
                  'n_2d_finder>=1&&n_gc>=1&&BhabahVeto==0&&SBhabhaVeto==0',
                  'n_c>=3&&n_high_c1>=1&&eclBhabhaVeto==0', 'n_c>=2&&bbc>=1&&eclBhabhaVeto==0',
                  'n_high_c2>=1&&eclBhabhaVeto==0', 'n_c>=2&&n_bbc>=1', 'n_2d_finder>=1&&n_gc>=1', 'n_high_c2>=1',
                  'eclbhabha', 'n_2d_finder>=1&&n_high_c3>=1&&n_bbtc>=1', 'n_high_c3>=1&&n_2d_finder==0']
        if self.Ntot_event != 0:
            for i in range(ntrg):
                eff = self.Nsubtrg_event[i] / self.Ntot_event
                print('T%3d                %4d              %6.2f          %s ' % (i, prescale[i], eff, trglog[i]))


def EffCalculation(path):
    path.add_module(EffModule())

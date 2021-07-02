#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
# This steering file generates events and uses python to draw
# histograms of PID information.
#
# Usage: basf2 plot_LL_diff.py
#
# Input: None
# Output: ll_diff.png
#############################################################

import basf2 as b2

from ROOT import Belle2
from ROOT import TCanvas, TH2F
from simulation import add_simulation
from reconstruction import add_reconstruction

# the histograms to fill
hist = [TH2F(
    'hist' + str(i),
    'hist' + str(i),
    50,
    0.0,
    3.5,
    50,
    -100,
    100,
) for i in range(4)]

hist[0].SetTitle('True pions, dE/dx')
hist[1].SetTitle('True kaons, dE/dx')
hist[2].SetTitle('True pions, TOP')
hist[3].SetTitle('True kaons, TOP')

# define the python module to extract and plot PID information


class MinModule(b2.Module):

    """
    Get LL differences from PIDLikelihood and fill them into histograms
    """

    def __init__(self):
        """
        call constructor of base class, required.
        """

        super(MinModule, self).__init__()

    def event(self):
        """
        reimplement Module::event()
        """

        pids = Belle2.PyStoreArray('PIDLikelihoods')
        for pid in pids:
            track = pid.getRelatedFrom('Tracks')
            if track:
                mcpart = track.getRelatedTo('MCParticles')
            if not track or not mcpart:
                # some tracks don't have an mcparticle
                b2.B2WARNING('problems with track <-> mcparticle relations')
                event = Belle2.PyStoreObj('EventMetaData').obj().getEvent()
                print('event: %d, track: %d' % (event, track.getArrayIndex()))
            else:
                pdg = abs(mcpart.getPDG())
                momentumVec = mcpart.getMomentum()
                momentum = momentumVec.Mag()
                if momentum > 3.5:  # cut off
                    continue
                # theta = momentumVec.CosTheta()

                # particle to compare with pions
                selectedpart = Belle2.Const.kaon
                pid_dedx = Belle2.Const.PIDDetectorSet(Belle2.Const.CDC)
                pid_top = Belle2.Const.PIDDetectorSet(Belle2.Const.TOP)
                logl_sel = pid.getLogL(selectedpart, pid_dedx)
                logl_pi = pid.getLogL(Belle2.Const.pion, pid_dedx)
                dedx_DLL = logl_pi - logl_sel

                logl_sel = pid.getLogL(selectedpart, pid_top)
                logl_pi = pid.getLogL(Belle2.Const.pion, pid_top)
                top_DLL = logl_pi - logl_sel

                if pdg == selectedpart.getPDGCode():
                    hist[0].Fill(momentum, dedx_DLL)
                    hist[2].Fill(momentum, top_DLL)
                elif pdg == 211:
                    hist[1].Fill(momentum, dedx_DLL)
                    hist[3].Fill(momentum, top_DLL)

    def terminate(self):
        """
        Draw histograms on canvas and save image.
        """

        c1 = TCanvas('c', 'c', 800, 600)
        c1.Divide(2, 2)
        c1.Show()
        for i in range(4):
            c1.cd(i + 1)
            hist[i].Draw('colz')
        # c1.Draw()

        c1.SaveAs('ll_diff.png')
        print('Output plots saved in ll_diff.png')


# create path
main = b2.create_path()

# define number of events
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [20])
main.add_module(eventinfosetter)
main.add_module('EvtGenInput')

# do full simulation and reconstruction
add_simulation(main)
add_reconstruction(main)

# draw histograms of log likelihood differences
main.add_module(MinModule())

# process events and print call statistics
b2.process(main)
print(b2.statistics)

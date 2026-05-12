##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
This test checks that the CATFinder module wrks.
In case of changes in the simulation, the parameters of the MCParticles might need to be adapted.
'''

import basf2 as b2
import simulation as si
from tracking.path_utils import add_cdc_track_finding

import math


class AddMCParticle(b2.Module):
    '''Simple module that add a specific MCParticle per event.'''

    def initialize(self):
        '''Initialize.'''
        import ROOT  # noqa
        self.mc_particles = ROOT.Belle2.PyStoreArray('MCParticles')
        self.mc_particles.registerInDataStore()
        self.event_number = 0

    def event(self):
        '''Event.'''
        import ROOT  # noqa
        mc_particle = self.mc_particles.appendNew()
        if self.event_number == 0:
            # 4 GeV positive muon, phi=20, theta=80 degrees
            pdg = -13
            p = 4.0
            phi = math.radians(20)
            theta = math.radians(80)
        else:
            # 2 GeV negative pion, phi=90, theta=105 degrees
            pdg = -211
            p = 2.0
            phi = math.radians(90)
            theta = math.radians(105)
        momentum = ROOT.Math.XYZVector(
            p * math.sin(theta) * math.cos(phi),
            p * math.sin(theta) * math.sin(phi),
            p * math.cos(theta)
        )
        vertex = ROOT.Math.XYZVector(0, 0, 0)
        mc_particle.setPDG(pdg)
        mc_particle.setMassFromPDG()
        mc_particle.addStatus(ROOT.Belle2.MCParticle.c_PrimaryParticle)
        mc_particle.addStatus(ROOT.Belle2.MCParticle.c_StableInGenerator)
        mc_particle.setProductionVertex(vertex)
        mc_particle.setMomentum(momentum)
        m = mc_particle.getMass()
        mc_particle.setEnergy(math.sqrt(p*p + m*m))
        mc_particle.setDecayTime(float("inf"))

        self.event_number += 1


class CheckRecoTrack(b2.Module):
    '''Simple module that checks a RecoTrack.'''

    def initialize(self):
        '''Initialize.'''
        import ROOT  # noqa
        self.reco_tracks = ROOT.Belle2.PyStoreArray('RecoTracks')
        self.reco_tracks.isRequired()
        self.event_number = 0

    def event(self):
        '''Event.'''
        assert (self.reco_tracks.getEntries() == 1)
        reco_track = self.reco_tracks[0]
        assert (reco_track.getNumberOfCDCHits() == reco_track.getNumberOfTotalHits())
        hits = reco_track.getNumberOfTotalHits()
        position = reco_track.getPositionSeed()
        momentum = reco_track.getMomentumSeed()
        charge = reco_track.getChargeSeed()
        fit = reco_track.wasFitSuccessful()
        assert (hits >= 40)
        assert (math.sqrt(position.Mag2()) < 10.0)
        assert (fit)
        if self.event_number == 0:
            assert (3.5 <= math.sqrt(momentum.Mag2()) <= 4.5)
            assert (charge > 0)
        else:
            assert (1.5 <= math.sqrt(momentum.Mag2()) <= 2.5)
            assert (charge < 0)

        self.event_number += 1


b2.set_random_seed('cat')


main = b2.Path()

main.add_module('EventInfoSetter', evtNumList=2)

main.add_module('Progress')

main.add_module(AddMCParticle())

si.add_simulation(main)

add_cdc_track_finding(main, use_cat_finder=True)

main.add_module('StatisticsSummary').set_name('Sum_CATFinder')

main.add_module('SetupGenfitExtrapolation')

main.add_module('DAFRecoFitter')

main.add_module(CheckRecoTrack())

b2.print_path(main)
b2.process(main, calculateStatistics=True)

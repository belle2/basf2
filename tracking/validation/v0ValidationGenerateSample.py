#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>tracking@belle2.kek.jp</contact>
  <output>V0ValidationSample.root, V0ValidationHarvested.root</output>
  <description>This module generates events for the V0 validation.</description>
</header>
"""

import basf2
from simulation import add_simulation
from reconstruction import add_mdst_output
from modularAnalysis import generateY4S
from tracking import add_track_finding, add_prune_tracks
from tracking.validation.harvesting import HarvestingModule
from tracking.validation import refiners, tracking_efficiency_helpers
import numpy


def run():
    components = tracking_efficiency_helpers.get_simulation_components()

    basf2.set_random_seed(1337)
    path = basf2.create_path()

    # generateY4S(1000, path=path)

    path.add_module('EventInfoSetter',
                    evtNumList=[1000],
                    runList=[1],
                    expList=[1]
                    )

    path.add_module('ParticleGun',
                    pdgCodes=[310],
                    nTracks=5,
                    momentumGeneration='uniform',
                    momentumParams=[0.000, 1.000],
                    thetaGeneration='uniform',
                    thetaParams=[17, 150],
                    phiGeneration='uniform',
                    phiParams=[0, 360],
                    vertexGeneration='uniform',
                    xVertexParams=[0.0, 0.0],
                    yVertexParams=[0.0, 0.0],
                    zVertexParams=[0.0, 0.0]
                    )

    path.add_module('Gearbox')
    add_simulation(path, components=components)

    # Material effects for all track extrapolations
    path.add_module('SetupGenfitExtrapolation')

    add_track_finding(path, components)

    # Match the tracks to the MC truth.  The matching works based on
    # the output of the TrackFinderMCTruth.
    path.add_module('TrackFinderMCTruth', GFTrackCandidatesColName='MCTrackCands', WhichParticles=[])
    path.add_module('MCMatcherTracks', MCGFTrackCandsColName='MCTrackCands')

    # track fitting
    path.add_module('GenFitter', PDGCodes=[211]).set_name('combined GenFitter')

    # create Belle2 Tracks from the genfit Tracks
    path.add_module('TrackBuilder')

    # V0 finding
    path.add_module('V0Finder', Validation=True)
    path.add_module('MCV0Matcher', V0ColName='V0ValidationVertexs')
    path.add_module(V0Harvester())

    # prune genfit tracks
    add_prune_tracks(path)

    # Store mdst output plus v0validation
    add_mdst_output(path, filename='../V0ValidationSample.root', additionalBranches=['V0Validations'])

    basf2.process(path)
    print(basf2.statistics)


class V0Harvester(HarvestingModule):

    def __init__(self):
        HarvestingModule.__init__(self, foreach="MCParticles", output_file_name="../V0ValidationHarvested.root")

    def pick(self, mc_particle):
        return abs(mc_particle.getPDG()) == 310

    def peel(self, mc):
        mc_vertex = mc.getDecayVertex()
        mc_perp = mc_vertex.Perp()
        mc_theta = mc_vertex.Theta()
        mc_phi = mc_vertex.Phi()
        mc_m = mc.getMass()
        mc_p = mc.getMomentum().Mag()

        v0 = mc.getRelated("V0ValidationVertexs")

        if v0:
            v0_vertex = v0.getVertexPosition()
            v0_perp = v0_vertex.Perp()
            v0_theta = v0_vertex.Theta()
            v0_phi = v0_vertex.Phi()
            v0_m = v0.getFittedInvariantMass()
            v0_p = v0.getFittedMomentum()
            v0_chi2 = v0.getVertexChi2()

            return {
                "R": v0_perp,
                "R_MC": mc_perp,
                "THETA": v0_theta,
                "THETA_MC": mc_theta,
                "PHI": v0_phi,
                "PHI_MC": mc_phi,
                "P": v0_p,
                "P_MC": mc_p,
                "M": v0_m,
                "M_MC": mc_m,
                "CHI2": v0_chi2,
                "FOUND": True
            }

        else:
            return {
                "R": numpy.NaN,
                "R_MC": mc_perp,
                "THETA": numpy.NaN,
                "THETA_MC": mc_theta,
                "PHI": numpy.NaN,
                "PHI_MC": mc_phi,
                "P": numpy.NaN,
                "P_MC": mc_p,
                "M": numpy.NaN,
                "M_MC": mc_m,
                "CHI2": numpy.NaN,
                "FOUND": False
            }

    save_tree = refiners.SaveTreeRefiner()


if __name__ == '__main__':
    run()

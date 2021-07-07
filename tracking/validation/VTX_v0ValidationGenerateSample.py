#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>VTX_KShortGenSimNoBkg.root</input>
  <output>VTX_V0ValidationSample.root, VTX_V0ValidationHarvested.root</output>
  <description>This module generates events for the V0 validation for Belle II w/ VTX..</description>
</header>
"""

import basf2
from tracking import add_tracking_reconstruction
from tracking.harvest.harvesting import HarvestingModule
from tracking.harvest.refiners import SaveTreeRefiner
import numpy


class V0Harvester(HarvestingModule):
    """Collects variables of interest for the V0Validation and the v0ValidationCreatePlots script."""

    def __init__(self):
        """Initialize the harvester.
        Defines over which StoreArray is iterated and the output file.
        """
        HarvestingModule.__init__(self, foreach="MCParticles", output_file_name="../VTX_V0ValidationHarvested.root")

    def pick(self, mc_particle):
        """Selects all MCParticles which are KShort.

        :param mc_particle: Belle2::MCParticle.
        :return: True if the MCParticle is a KShort.
        """
        return abs(mc_particle.getPDG()) == 310

    def peel(self, mc):
        """Selects MCTrue variables of interest for all KShort in the sample. If the KShort has a related reconstructed
        V0, these values are written out too. Variables of interest are:
        R: Radial (in xy) distance to origin.
        Theta: Theta Angle of decay vertex.
        Phi: Phi Angle of decay vertex.
        P: Momentum of the KShort.
        M: Invariant mass of the KShort.
        Chi2: Chi2 of vertex fit.
        isFound: True if MCParticle has a related V0.

        If the MCParticle has no related V0, the variables are filled with NaN's.

        :param mc: Belle2::MCParticle
        :return: dict with the variables of interest.
        """
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

    #: Refiners to be executed at the end of the harvesting / termination of the module
    #: Store the output of the module in a root file.
    save_tree = SaveTreeRefiner()


basf2.set_random_seed(1337)
path = basf2.create_path()

path.add_module('RootInput', inputFileName='../VTX_KShortGenSimNoBkg.root')
path.add_module('Gearbox')

add_tracking_reconstruction(path, useVTX=True)

# Set options for V0 Validation
for module in path.modules():
    if module.name() == "V0Finder":
        module.param("Validation", True)
path.add_module('MCV0Matcher', V0ColName='V0ValidationVertexs')
path.add_module(V0Harvester())

basf2.process(path)
print(basf2.statistics)

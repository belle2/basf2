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
  <contact>benni</contact>
  <description>
    Plot the efficiency to find a cluster for a given truehit for all layers of
    the VTX.
  </description>
</header>
"""
import reconstruction as rec
import simulation as sim
from vtx import get_upgrade_globaltag
from ROOT import Belle2
import ROOT
import sys
import math
import basf2 as b2
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = 1
b2.set_log_level(b2.LogLevel.ERROR)
# set some random seed
b2.set_random_seed(10346)

# momenta to generate the plots for
momenta = [3.0]
# theta parameters
theta_params = [90, 0.1]


class ClusterEfficiency(b2.Module):
    """
    Plot Efficiency to find a cluster for each given truehit.
    """

    def initialize(self):
        """
        Create ROOT TProfiles for all layers and momenta.
        """

        # let's create a root file to store all profiles
        #: Output file to store all plots
        self.rfile = ROOT.TFile("ClusterEfficiency.root", "RECREATE")
        self.rfile.cd()
        #: flat list of all profiles for easy access
        self.profiles = []
        #: layer/momentum hierachy of all profiles
        self.eff = {}

        def profile(name, title, text, contact):
            """small helper function to create a phi profile and set the names
            and descriptions"""
            prof = ROOT.TProfile(name, title, 60, -180, 180)
            prof.GetListOfFunctions().Add(ROOT.TNamed("Description", text))
            prof.GetListOfFunctions().Add(ROOT.TNamed("Contact", contact))
            prof.GetListOfFunctions().Add(ROOT.TNamed("Check", "Should be close to 1 everywhere"))
            # make a list of all profiles
            self.profiles.append(prof)
            return prof

        # name, title and decription templates
        prof_name = "ClusterEfficiency_layer{layer}_{p:.1f}GeV"
        prof_title = "Cluster Efficiency in #phi, layer={layer}, "\
            + "p={p:.1f} GeV;#phi in degrees;efficiency"
        prof_text = "Efficiency to find a vtx cluster for any given truehit "\
            + "in layer {layer} when simulation muons with p={p:.1f} GeV uniformly "\
            + "in phi and theta={theta[0]}+-{theta[1]} degree. phi is the angle "\
            + "of the generated particle, not the truehit position."
        # contact person for VTX
        prof_contact = "Benjamin Schwenker <Benjamin.Schwenker@phys.uni-goettingen.de>"

        # FIXME: better to avoid harcoding number of layers here for VTX
        # create all profiles
        for layer in range(1, 8):
            self.eff[layer] = {}
            for p in momenta:
                name = prof_name.format(p=p, layer=layer)
                title = prof_title.format(p=p, layer=layer)
                text = prof_text.format(p=p, layer=layer, theta=theta_params)
                self.eff[layer][p] = profile(name, title, text, prof_contact)

    def terminate(self):
        """
        Format all profiles and write the ROOT file.
        """
        # determine min value in all profiles to set all profiles to a common
        # range. We always want to show at least the range from 0.9
        minVal = 0.9
        for prof in self.profiles:
            minBin = prof.GetMinimumBin()
            # minimum value is bin content - error and a 5% margin
            minVal = min(minVal, (prof.GetBinContent(minBin) - prof.GetBinError(minBin)) * 0.95)

        # no let's set the range of all profiles to be equal
        for prof in self.profiles:
            prof.SetMinimum(max(0, minVal))
            prof.SetMaximum(1.02)
            prof.SetStats(False)

        self.rfile.Write()
        self.rfile.Close()

    def fill_truehits(self, phi, p, truehits):
        """
        Loop over all truehits for a track with the given angle phi and momentum
        p and update the efficiency profiles.
        """
        # iterate over all truehits and look for the clusters
        for i, truehit in enumerate(truehits):
            # we ignore all truehits which have an negative weight
            if truehits.weight(i) < 0:
                continue

            # get layer number and a list of all clusters related to the truehit
            layer = truehit.getSensorID().getLayerNumber()
            if isinstance(truehit, Belle2.VTXTrueHit):
                is_vtx = True
                clusters = truehit.getRelationsFrom("VTXClusters")

            # now check if we find a cluster
            has_cluster = {False: 0, True: 0}
            for j, cls in enumerate(clusters):
                # we ignore all clusters where less then 100 electrons come from
                # our truehit
                if clusters.weight(j) < 100:
                    continue

                if is_vtx:
                    # for vtx we always have both directions so set efficiency
                    # for this TrueHit to 1
                    has_cluster[True] = 1
                    has_cluster[False] = 1

            # and we fill the profile
            self.eff[layer][p].Fill(phi, has_cluster[True] & has_cluster[False])

    def event(self):
        """
        Update the efficienies by iterating over all primary particles
        """
        mcparticles = Belle2.PyStoreArray("MCParticles")
        for mcp in mcparticles:
            # we only look at primary particles
            if not mcp.hasStatus(1):
                continue

            # let's get the momentum and determine which of the ones we wanted
            # to generate it is
            p = mcp.getMomentum()
            p_gen = None
            for i in momenta:
                if abs(p.Mag() - i) < 0.05:
                    p_gen = i
                    break

            # meh, something strange with the momentum, ignore this one
            if p_gen is None:
                B2WARNING("Strange particle momentum: %f, expected one of %s" %
                          (p.Mag(), ", ".join(str() for p in momenta)))
                continue

            # and check all truehits
            vtxtruehits = mcp.getRelationsTo("VTXTrueHits")
            self.fill_truehits(math.degrees(p.Phi()), p_gen, vtxtruehits)


# Now let's create a path to simulate our events. We need a bit of statistics but
# that's not too bad since we only simulate single muons
main = b2.create_path()

b2.conditions.disable_globaltag_replay()
b2.conditions.prepend_globaltag(get_upgrade_globaltag())

main.add_module("EventInfoSetter", evtNumList=[10000])

main.add_module('Gearbox')
main.add_module('Geometry')

particlegun = main.add_module("ParticleGun")
particlegun.param({
    "nTracks": 1,
    "pdgCodes": [13, -13],
    # generate discrete momenta with equal weights
    "momentumGeneration": 'discrete',
    "momentumParams": momenta + [1] * len(momenta),
    "thetaGeneration": 'normal',
    "thetaParams": theta_params,
})

# Add simulation modules
sim.add_simulation(main, components=['VTX'], useVTX=True)

# Add mc reconstruction
rec.add_mc_reconstruction(main, components=['VTX'], pruneTracks=False, useVTX=True)


clusterefficiency = ClusterEfficiency()
main.add_module(clusterefficiency)
main.add_module("Progress")

b2.process(main)
print(b2.statistics)

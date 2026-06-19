#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################################
#
# This steering file expects a simulated data sample at innput and
# performs the standard reconstruction. The MillepedeCollector
# is used to re-fit tracks with General Broken Lines
#
# Finally the display is run to demonstrate its capability to visualize tracks
# fitted with GBL (done as byproduct of MillepedeCollector module)
#
##############################################################################
##

import basf2 as b2
import simulation
import reconstruction

import modularAnalysis as ana

import ROOT

b2.set_random_seed(42)


ROOT.gROOT.SetBatch(0)

main = b2.create_path()


# main.add_module("RootInput")


main.add_module('EventInfoSetter')
# main.add_module('EvtGenInput')
main.add_module('HistoManager', histoFileName='CollectorOutput.root')
main.add_module('Gearbox')
main.add_module('Geometry')
main.add_module('ParticleGun', momentumParams=[5.5, 5.50000000001], nTracks=1, pdgCodes=[
                13])  # thetaParams=[90., 90.0000000000001], phiParams=[0., 0.00000001]
simulation.add_simulation(main)
reconstruction.add_reconstruction(main, pruneTracks=False, add_muid_hits=False, components=["PXD", "SVD", "CDC"])


new_main = b2.create_path()

for mod in main.modules():
    if mod.type() == "DAFRecoFitter":
        gbl_mod = b2.register_module("GBLRecoFitter4")
        gbl_mod.param("resolveAmbiguities", 1)
        gbl_mod.param("externalIterations", 2)
        gbl_mod.param("gblInternalIterations", "hh")
        gbl_mod.set_name(str(mod.name()).replace("DAFRecoFitter", "GBLRecoFitter4"))
        for par in mod.available_params():
            if par.setInSteering:
                gbl_mod.param(par.name, par.values)
        print("Replacing DAFRecoFitter with name:", mod.name(), "with GBLRecoFitter using name:", gbl_mod.name())
        new_main.add_module(gbl_mod)
    else:
        new_main.add_module(mod)

main = new_main


new_main = b2.create_path()

for mod in main.modules():

    if mod.name() == "Combined_DAFRecoFitter":
        new_main.add_module("GBLRecoFitter4", resolveAmbiguities=1, externalIterations=2, gblInternalIterations="hh,hh")
    else:
        new_main.add_module(mod)

main = new_main


# main.add_module('GBLRecoFitter4')
"""
main.add_module(
    'MillepedeCollector',
    components=[],
    tracks=['RecoTracks'])
"""

# main.add_module('Display', showRecoTracks=True)


ana.fillParticleList("pi+:allCharged", "", path=main)

track_variables = [
    'chiProb',
    'd0',
    'z0',
    'phi0',
    'omega',
    'tanLambda',
    'pt',
    'pionID',
    'protonID',
    'electronID',
    'muonID',
    'deuteronID',
    'nVXDHits',
    'nPXDHits',
    'nSVDHits',
    'nCDCHits',
    'nTracks',
    'x',
    'y',
    'z']


ana.fillParticleList(
    'pi+:goodPval',
    'chiProb>0.5',
    path=main)
# main.add_module('SkimFilter', particleLists=['pi+:goodPval']).if_false(b2.create_path())

ana.variablesToNtuple("pi+:allCharged",
                      variables=track_variables,
                      filename='ntuples.root', path=main)

main.add_module('AlignDQM')
main.add_module('TrackDQM')


main.add_module("Progress", maxN=0)
b2.print_path(main)
b2.process(main)
print(b2.statistics)

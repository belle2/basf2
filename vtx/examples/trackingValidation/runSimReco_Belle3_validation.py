#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################################
#
# This steering file creates the Belle II detector geometry and perfoms the
# simulation and standard reconstruction and tuple production. The tuples
# will be used for resolution studies.
#
# This script produces tuples for nominal Belle II where PXD+SVD are replaced
# with VTX.
#
# Usage: basf2 runSimReco_Belle3_validation.py -n 10000 -- --gen "dimuon" (--globaltag "False")
##############################################################################


import variables.collections
from tracking.harvesting_validation.combined_module import CombinedTrackingValidationModule
import argparse
import os
import basf2 as b2
import simulation as sim
import reconstruction as rec
import modularAnalysis as ana
import vertex as vx
from ROOT import Belle2

import ROOT
ROOT.gROOT.SetBatch(0)

ap = argparse.ArgumentParser()
ap.add_argument("-g", "--gen", default='dimuon', help="Generator: 'gun', 'dimuon' or 'bbar'")
ap.add_argument("-gt", "--globaltag", type=str, choices=["True", "False"], default="True", help="Use Global tag or not")
args = vars(ap.parse_args())

# Necessary as argparse with bool is missleading
use_globaltag = args['globaltag']
if use_globaltag == "False":
    use_globaltag = False
else:
    use_globaltag = True


# Need to use default global tag prepended with upgrade GT
if use_globaltag:
    from vtx import get_upgrade_globaltag
    b2.conditions.disable_globaltag_replay()
    b2.conditions.prepend_globaltag(get_upgrade_globaltag())

main = b2.create_path()
main.add_module('EventInfoSetter')

if args['gen'] == 'bbar':
    main.add_module('EvtGenInput')
elif args['gen'] == 'dimuon':
    kkgeninput = b2.register_module('KKGenInput')
    kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat'))
    kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
    kkgeninput.param('taudecaytableFile', '')
    kkgeninput.param('kkmcoutputfilename', 'kkmc_mumu.txt')
    main.add_module(kkgeninput)
else:
    main.add_module('ParticleGun', pdgCodes=[-211, 211], momentumParams=[0.05, 6.0],
                    xVertexParams=[0.0],
                    yVertexParams=[0.0],
                    zVertexParams=[0.0]
                    )

main.add_module('Gearbox')

if use_globaltag:
    main.add_module('Geometry')

else:
    main.add_module('Geometry', excludedComponents=['PXD', 'SVD'],
                    additionalComponents=['VTX-CMOS-7layer'],
                    useDB=False)

# Add simulation modules
sim.add_simulation(main, useVTX=True)

# Add mc reconstruction
rec.add_mc_reconstruction(main, pruneTracks=False, useVTX=True)

main.add_module('DAFRecoFitter')


main.add_module(CombinedTrackingValidationModule(
                output_file_name="CombinedTrackingValidationModule_{}.root".format(args['gen']),
                reco_tracks_name="RecoTracks",
                mc_reco_tracks_name="MCRecoTracks",
                name="", contact="", expert_level=200))

main.add_module('TrackingPerformanceEvaluation', outputFileName="TrackingPerformanceEvaluation_{}.root".format(args['gen']))

track_variables = [
    'p',
    'E',
    'theta',
    'phi',
    'pErr',
    'ptErr',
    'thetaErr',
    'mcP',
    'mcPT',
    'mcTheta',
    'mcPhi',
    'phiErr',
    'mcPrimary',
    'mcInitial',
    'mcVirtual',
    'isSignal',
    'isCloneTrack',
    'PDG',
    'd0',
    'z0',
    'phi0',
    'omega',
    'tanlambda',
    'pt',
    'pionID',
    'protonID',
    'electronID',
    'muonID',
    'deuteronID',
    'firstPXDLayer',
    'firstSVDLayer',
    'firstVTXLayer',
    'nVXDHits',
    'nVTXHits',
    'nPXDHits',
    'nSVDHits',
    'nCDCHits',
    'nTracks',
    'chiProb']

# two track events with vertex constraint
ana.fillParticleList(
    'mu+:mu_dimuon',
    'abs(formula(z0)) < 0.5 and abs(d0) < 0.5 and nTracks == 2 and muonID > 0.7',
    writeOut=True,
    path=main)
ana.variablesToNtuple('mu+:mu_dimuon', variables=track_variables, filename='belle3_analysis_mu_dimuon.root', path=main)
ana.reconstructDecay('Z0:mumu -> mu-:mu_dimuon mu+:mu_dimuon', '', writeOut=True, path=main)
vx.treeFit('Z0:mumu', conf_level=0.001, updateAllDaughters=True, ipConstraint=False, path=main)
ana.variablesToNtuple(
    'Z0:mumu',
    variables=[
        'chiProb',
        'nTracks',
        'daughterDiffOf(0, 1, d0)',
        'daughterDiffOf(0, 1, z0)',
        'InvM'] +
    variables.collections.vertex,
    filename='belle3_analysis_dimuons.root',
    path=main)
ana.rankByHighest(particleList='Z0:mumu', variable='chiProb', cut='nTracks == 2', numBest=1, path=main)


ana.fillParticleList('pi+:all_charged', 'abs(formula(z0)) < 0.5 and abs(d0) < 0.5', writeOut=True, path=main)
ana.variablesToNtuple(
    'pi+:all_charged',
    variables=track_variables,
    filename='belle3_analysis_{}_all_charged.root'.format(
        args['gen']),
    path=main)


main.add_module('Progress')
b2.process(main)
print(b2.statistics)

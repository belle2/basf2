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
# NOTE: You MUST set environment variable BELLE2_VTX_UPGRADE_GT before running.
#
# Usage: basf2 runSimReco_Belle3.py -n 2000 -- --gen "dimuon"
##############################################################################


import variables.collections
from vtx import get_upgrade_globaltag
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
args = vars(ap.parse_args())

# Need to use default global tag prepended with upgrade GT
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
main.add_module('Geometry')

# Add simulation modules
sim.add_simulation(main, useVTX=True)

# Add mc reconstruction
rec.add_mc_reconstruction(main, pruneTracks=False, useVTX=True)

main.add_module('DAFRecoFitter')

track_variables = [
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
ana.variablesToNtuple('mu+:mu_dimuon', variables=track_variables,
                      filename='belle3_analysis_mu_dimuon_{}.root'.format(get_upgrade_globaltag()),
                      path=main)
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
    filename='belle3_analysis_dimuons_{}.root'.format(get_upgrade_globaltag()),
    path=main)
ana.rankByHighest(particleList='Z0:mumu', variable='chiProb', cut='nTracks == 2', numBest=1, path=main)


ana.fillParticleList('pi+:all_charged', 'abs(formula(z0)) < 0.5 and abs(d0) < 0.5', writeOut=True, path=main)
ana.variablesToNtuple(
    'pi+:all_charged',
    variables=track_variables,
    filename='belle3_analysis_{}_all_charged_{}.root'.format(
        args['gen'], get_upgrade_globaltag()),
    path=main)


main.add_module('Progress')
b2.process(main)
print(b2.statistics)

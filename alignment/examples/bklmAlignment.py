#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
from simulation import add_simulation
from reconstruction import add_mc_reconstruction
import glob

from ROOT import Belle2

reset_database()
use_local_database()

main = create_path()

input = register_module('RootInput')
input.param('inputFileName', 'RootOutput.root')
input.param('excludeBranchNames', ['GF2TracksToMCParticles', 'TrackCandsToGF2Tracks'])
input.initialize()

gear = register_module('Gearbox')
gear.initialize()

geometry = register_module('Geometry')
geometry.param({
    "excludedComponents": ["MagneticField"],
    "additionalComponents": ["MagneticField2d"],
})
geometry.initialize()

algo = Belle2.MillepedeAlgorithm()
algo.steering().command('method diagonalization 1 0.1')
algo.steering().command('entries 100')
algo.steering().command('chiscut 30. 6.')
algo.steering().command('outlierdownweighting 3')
algo.steering().command('dwfractioncut 0.1')

algo.steering().command('Parameters')
for vxdid in Belle2.VXD.GeoCache.getInstance().getListOfSensors():
    label = Belle2.GlobalLabel(vxdid, 0)
    for ipar in range(1, 7):
        par_label = label.label() + ipar
        cmd = str(par_label) + ' 0. -1.'
        algo.steering().command(cmd)

for icLayer in range(0, 57):
    cmd = str(Belle2.GlobalLabel(Belle2.WireID(icLayer, 511), 1).label()) + ' 0. -1.'
    algo.steering().command(cmd)
    cmd = str(Belle2.GlobalLabel(Belle2.WireID(icLayer, 511), 2).label()) + ' 0. -1.'
    algo.steering().command(cmd)

# fix everything except layer 10, shifts U, V
barrel = 1
for sector in range(0, 9):
    for layer in range(1, 17):
        for forward in [0, 1]:
            pars = [1, 2, 3, 4, 5, 6]
            for ipar in pars:
                if layer == 10 and ipar in [1, 2]:
                    continue
                klmid = layer + 100 * sector + 1000 * forward + 10000 * barrel
                label = Belle2.GlobalLabel(klmid, ipar)
                cmd = str(label.label()) + ' 0. -1.'
                algo.steering().command(cmd)

algo.execute()

import interactive
interactive.embed()

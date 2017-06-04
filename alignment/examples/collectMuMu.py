#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
use_local_database('localdb/database.txt')

import beamparameters as beam
import simulation as sim
import reconstruction as reco
import modularAnalysis as ana

main = create_path()

main.add_module("RootInput")
main.add_module("Gearbox")
main.add_module("Geometry")
reco.add_reconstruction(main, pruneTracks=False)
ana.fillParticleList('mu+:qed', 'muid > 0.1 and useLabFrame(p) > 2.', True, main)
ana.reconstructDecay('Z0:mumu -> mu-:qed mu+:qed', '', writeOut=True, path=main)
ana.vertexRaveDaughtersUpdate('Z0:mumu', 0.0, path=main, constraint='ipprofile')

main.add_module(
    "MillepedeCollector",
    useGblTree=False,
    minPValue=0.,
    primaryVertices=['Z0:mumu'],
    tracks=[],
    components=['VXDAlignment'])

main.add_module("Progress")
main.add_module('RootOutput', branchNames=['EventMetaData'])

process(main)
print(statistics)

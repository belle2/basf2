#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################
# Example script to demonstrate the NtupleMaker module
# (inspired by myBtoDpi.py)
#
# Christian Oswald (oswald@physik.uni-bonn.de), Uni Bonn, 2013.
###########################################################

import sys
import os
from basf2 import *

# Create main path
main = create_path()

# specify number of events to be generated in job
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('EvtNumList', [10])  # process 10 events
evtmetagen.param('RunList', [1])  # from run number 1
evtmetagen.param('ExpList', [1])  # and experiment number 1
main.add_module(evtmetagen)

# ----> EvtGen
evtgeninput = register_module('EvtGenInput')
# boost all momenta to LAB system
evtgeninput.param('boost2LAB', True)
# use specified user decay file
evtgeninput.param('userDECFile', os.environ['BELLE2_LOCAL_DIR']
                  + '/analysis/modules/B2Dpi/datafiles/BtoDpi.dec')
main.add_module(evtgeninput)

# ----> Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# ----> create geometry
gearbox = register_module('Gearbox')
main.add_module(gearbox)
geometry = register_module('Geometry')
geometry.param('Components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD', 'CDC'
               ])
main.add_module(geometry)

# ----> simulation
g4sim = register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR
main.add_module(g4sim)

# ----> CDC digitizer
cdcDigitizer = register_module('CDCDigitizer')
main.add_module(cdcDigitizer)

# ----> VXD digitizer
pxd_digi = register_module('PXDDigitizer')
main.add_module(pxd_digi)
pxd_cls = register_module('PXDClusterizer')
main.add_module(pxd_cls)
svd_digi = register_module('SVDDigitizer')
main.add_module(svd_digi)
svd_cls = register_module('SVDClusterizer')
main.add_module(svd_cls)

# ----> MC Trackfinder
mctrackfinder = register_module('MCTrackFinder')
mctrackfinder.param('UseCDCHits', True)
mctrackfinder.param('UseSVDHits', True)
mctrackfinder.param('UsePXDHits', True)
mctrackfinder.param('GFTrackCandidatesColName', 'GFTrackCands')
mctrackfinder.param('WhichParticles', ['primary'])
main.add_module(mctrackfinder)

# ----> VXD Track fitting is performed with GenFit
trackfitting = register_module('GenFitter')
main.add_module(trackfitting)

# ----> Load final state particles
particleloader = register_module('ParticleLoader')
main.add_module(particleloader)

# ----> Select kaons and pions
kaonselector = register_module('ParticleSelector')
kaonselector.param('PDG', -321)
kaonselector.param('ListName', 'k')
main.add_module(kaonselector)

pionselector = register_module('ParticleSelector')
pionselector.param('PDG', 211)
pionselector.param('ListName', 'pi')
main.add_module(pionselector)

# ----> Reco D0
combinerD0 = register_module('ParticleCombiner')
combinerD0.param('PDG', -421)
combinerD0.param('ListName', 'D0')
combinerD0.param('InputListNames', ['k', 'pi'])
main.add_module(combinerD0)

# ----> Reco B
combinerB = register_module('ParticleCombiner')
combinerB.param('PDG', 521)
combinerB.param('ListName', 'B')
combinerB.param('InputListNames', ['D0', 'pi'])
main.add_module(combinerB)

ntuple1 = register_module('NtupleMaker')
# output root file name (the suffix .root will be added automaticaly)
ntuple1.param('strFileName', 'test.root')
# Name of the output TTree
ntuple1.param('strTreeName', 'test1')
# particle list with reconstructed decays
ntuple1.param('strListName', 'B')
# Specifiy output tools
# decay descriptor strings ignored (parser for  not yet implemented)
ntuple1.param('strTools', [
    'EventMetaData',
    'B-',
    'Kinematics',
    'B- -> (anti-D0 -> ^K+ ^pi-) ^pi-',
    'MCKinematics',
    'B- -> (anti-D0 -> ^K+ ^pi-) ^pi-',
    ])
main.add_module(ntuple1)

# ----> start processing of modules
process(main)

# ----> Print call statistics
print statistics

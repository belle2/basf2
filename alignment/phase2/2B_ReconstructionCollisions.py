#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# *****************************************************************************

# title           : 2B_ReconstructionCollisions.py
# description     : Reconstruction collision events
# author          : Jakub Kandra (jakub.kandra@karlov.mff.cuni.cz)
# date            : 8. 2. 2018

# *****************************************************************************

from basf2 import *
from simulation import add_simulation
import tracking
import reconstruction
import modularAnalysis

inname = "generation.root"
outname = "reconstruction.root"

if len(sys.argv) == 3:
    inname = (sys.argv)[1]
    outname = (sys.argv)[2]

main = create_path()

main.add_module('RootInput', inputFileName=inname)

main.add_module('Gearbox', fileName='/geometry/Beast2_phase2.xml')

components = [
    'BeamPipe',
    'MagneticField',
    'PXD',
    'SVD',
    'CDC',
    'EKLM',
    'BKLM',
    'ECL']

# reconstruction
recoconstruction.add_reconstruction(path, pruneTracks=False, components=components)

# analysis to select vertex constraint decays and other muons
modularAnalysis.fillParticleList('mu+:bbmu', 'muonID > 0.1 and useLabFrame(p) < 4.5', True, main)
# Pre-fit with beam+vertex constraint decays for muon pairs
modularAnalysis.fillParticleList('mu+:qed', 'muonID > 0.1 and useCMSFrame(p) > 4.5', writeOut=True, path=main)
modularAnalysis.reconstructDecay('Z0:mumu -> mu-:qed mu+:qed', '', writeOut=True, path=main)
modularAnalysis.vertexRaveDaughtersUpdate('Z0:mumu', 0.0, path=main, constraint='ipprofile')

main.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)
main.add_module('HistoManager', histoFileName='CollectorOutput.root')
main.add_module('MillepedeCollector',
                components=['VXDAlignment', 'BeamParameters'],
                tracks=[],
                particles=['mu+:bbmu'],
                vertices=[],
                primaryVertices=['Z0:mumu'])

main.add_module('RootOutput', outputFileName=outname)

progress = register_module('ProgressBar')
main.add_module(progress)

process(main)

# Print call statistics
print(statistics)

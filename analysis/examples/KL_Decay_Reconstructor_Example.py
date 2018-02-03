#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
#
# This is a script that generates double generic BBbar
# events and performs a charmonium skim.
# Surviving events proceed to simulation and reconstruction.
# We then reconstruct B -> J/psi KL events and recalculate the
# KL momentum according to reconstructed direction using
# kinematical constraints.
#
# Contributors: B. Oberhof, benjamin.oberhof@lnf.infn.it
#
#############################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdLightMesons import *
from ROOT import Belle2
from vertex import vertexRave
from vertex import TagV
from reconstruction import add_mdst_output
from glob import glob
import basf2

# Set parameters
inputName = "/ghi/fs01/belle2/bdata/MC/signal/B2JpsiKL/JPsiKL.root"
outputName = "reconstruct_jpsi_kl_example.root"

inputMdstList('default', inputName)

# Show progress of processing
progress = register_module('ProgressBar')
analysis_main.add_module(progress)

stdLooseMu()
fillParticleList('K_L0:sig', 'E > 0.5')

reconstructDecay(
    'J/psi:mumu -> mu-:loose mu+:loose',
    ' 3.08 < M < 3.12  and useCMSFrame(p) > 1.45 and useCMSFrame(p) < 1.95',
    1,
    True)
matchMCTruth('J/psi:mumu')

pcalc = register_module('KLMomentumCalculator')
pcalc.set_name('KLMomentumCalculator_' + 'B0:1 -> J/psi:mumu K_L0:sig')
pcalc.param('decayString', 'B0:1 -> J/psi:mumu K_L0:sig')
pcalc.param('cut', '')
pcalc.param('decayMode', 0)
pcalc.param('writeOut', 1)
pcalc.param('recoList', "_reco")
analysis_main.add_module(pcalc)

rmake = register_module('KLDecayReconstructor')
rmake.set_name('KLDecayReconstructor_' + 'B0:1 -> J/psi:mumu K_L0:sig')
rmake.param('decayString', 'B0:1 -> J/psi:mumu K_L0:sig')
rmake.param('cut', 'M>0')
rmake.param('decayMode', 0)
rmake.param('writeOut', 1)
rmake.param('recoList', "_reco")
analysis_main.add_module(rmake)

buildRestOfEvent('B0:1')
matchMCTruth('B0:1')

TagV('B0:1', 'breco', 0.0001, 'standard_PXD')

toolsDST1 = ['EventMetaData', '^B0:1']
toolsDST1 += ['RecoStats', '^B0:1 -> J/psi:mumu K_L0:sig_reco']
toolsDST1 += ['InvMass', '^B0:1 -> ^J/psi:mumu ^K_L0:sig_reco']
toolsDST1 += ['DeltaEMbc', '^B0:1']
toolsDST1 += ['Kinematics', '^B0:1 -> ^J/psi:mumu ^K_L0:sig_reco']
toolsDST1 += ['MCKinematics', '^B0:1 -> ^J/psi:mumu ^K_L0:sig_reco']
toolsDST1 += ['CMSKinematics', '^B0:1 -> ^J/psi:mumu ^K_L0:sig_reco']
toolsDST1 += ['MCTruth', '^B0:1 -> ^J/psi:mumu ^K_L0:sig_reco']
toolsDST1 += ['DeltaT', '^B0:1']
toolsDST1 += ['MCDeltaT', '^B0:1']
toolsDST1 += ['PDGCode', '^B0:1']
toolsDST1 += ['Vertex', '^B0:1 -> ^J/psi:mumu ^K_L0:sig_reco']
toolsDST1 += ['MCVertex', '^B0:1 -> ^J/psi:mumu ^K_L0:sig_reco']
toolsDST1 += ['TagVertex', '^B0:1']
toolsDST1 += ['MCTagVertex', '^B0:1']
toolsDST1 += ['CustomFloats[isSignal:isExtendedSignal]', '^B0:1 -> ^J/psi:mumu ^K_L0:sig_reco']
toolsDST1 += ['CustomFloats[extraInfo(decayModeID)]', '^B0:1']

ntupleFile(outputName)
ntupleTree('B1tree', 'B0:1', toolsDST1)

set_log_level(LogLevel.ERROR)

# Process all modules added to the analysis_main path
process(analysis_main)

# Print out the summary
print(statistics)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to perform an splot training.
# The decay D0 -> K- pi+ is reconstructed and a data-driven
# splot training is performed using the fact that the signal
# candidates peak in the invariant mass around the D0 mass.
#
# Note: This example uses the generic MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
#
# Contributors: T. Keck (June 2015)
#
######################################################

from basf2 import *
from modularAnalysis import *

filelist = [
    '/local/ssd-scratch/tkeck/mc/mc35_charged_BGx0_s00/charged_e0001r000*.root',
    # '/hsm/belle2/bdata/MC/generic/mixed/mcprod1405/BGx1/mc35_mixed_BGx1_s00/mixed_e0001r0010_s00_BGx1.mdst.root',
    # '/hsm/belle2/bdata/MC/generic/charged/mcprod1405/BGx1/mc35_charged_BGx1_s00/charged_e0001r0010_s00_BGx1.mdst.root',
    # '/hsm/belle2/bdata/MC/generic/ccbar/mcprod1405/BGx1/mc35_ccbar_BGx1_s00/ccbar_e0001r0010_s00_BGx1.mdst.root',
    # '/hsm/belle2/bdata/MC/generic/ssbar/mcprod1405/BGx1/mc35_ssbar_BGx1_s00/ssbar_e0001r0010_s00_BGx1.mdst.root',
    # '/hsm/belle2/bdata/MC/generic/ddbar/mcprod1405/BGx1/mc35_ddbar_BGx1_s00/ddbar_e0001r0010_s00_BGx1.mdst.root',
    # '/hsm/belle2/bdata/MC/generic/uubar/mcprod1405/BGx1/mc35_uubar_BGx1_s00/uubar_e0001r0010_s00_BGx1.mdst.root'
]

main = create_path()
inputMdstList('MC5', filelist, path=main)

fillParticleList('K-', 'Kid > 0.1', path=main)
fillParticleList('pi+', 'piid > 0.1', path=main)

# the vertex fit changes the mass a bit; if we cut later, the bins
# at the edges will have fewer statistics because of that. Thus,
# take a bit more, 0.0025 GeV, on each side, and cut it after the fit.
reconstructDecay('D0:raw -> K- pi+', '1.8275 < M < 1.9025', path=main)
vertexKFit('D0:raw', 0.001, path=main)
# because the vertex fix changes the mass a bit
cutAndCopyList('D0', 'D0:raw', '1.83 < M < 1.9', path=main)
matchMCTruth('D0', path=main)

teacher = register_module('TMVATeacher')
teacher.param('workingDirectory', 'TMVA')
teacher.param('prefix', 'SPlot')
teacher.param('variables', ['daughter(0, Kid)', 'daughter(1, piid)',
                            'daughter(0, chiProb)', 'daughter(1, chiProb)',
                            'useRestFrame(daughter(0, p))',
                            'useRestFrame(daughter(1, p))',
                            'dr', 'dz', 'chiProb'])
# We include the discriminating variable M in the spectators
# and additionally the MC truth, of course you don't have the latter one on real data.
teacher.param('spectators', ['isSignal', 'M'])
teacher.param('listNames', 'D0')
main.add_module(teacher)

process(main)
print(statistics)

# Now one needs to define a sPlot model with RooFit
from ROOT import (RooRealVar, RooGaussian, RooChebychev, RooAddPdf, RooArgList, RooFit, RooAbsReal, TFile, kFALSE)

# Define observable, in this case we use the invariant mass M as discriminating variable
M = RooRealVar("M", "M", 1.83, 1.9)
M.setBins(50)

# Setup component PDFs for signal and background.
# The name sig and bkg for the PDFs are assumend in the TMVAInterface code! So use them!
mD0 = RooRealVar("mD0", "D0 Mass", 1.865)
sigmaD0 = RooRealVar("sigmaD0", "Width of Gaussian", 0.025)
sig = RooGaussian("sig", "D0 Model", M, mD0, sigmaD0)
mD0.setConstant(kFALSE)
sigmaD0.setConstant(kFALSE)

a0 = RooRealVar("a0", "a0", -0.69)
a1 = RooRealVar("a1", "a1", 0.1)
a0.setConstant(kFALSE)
a1.setConstant(kFALSE)
bkg = RooChebychev("bkg", "Background", M, RooArgList(a0, a1))

# The names signal and background for the PDF yields are assumend in the TMVAInterface code! So use them!
sigfrac = RooRealVar("signal", "fraction of signal", 0.5)
bkgfrac = RooRealVar("background", "fraction of background", 0.5)
bkgfrac.setConstant(kFALSE)
sigfrac.setConstant(kFALSE)

# The name model for the complete model is assumed in the TMVAInterface code! So use it!
model = RooAddPdf('model', "bkg+sig", RooArgList(bkg, sig), RooArgList(bkgfrac, sigfrac))

# Write model to file and close the file, so TMVASPlotTeacher can open it.
modelFile = TFile('TMVA/SPlotModel.root', "RECREATE")
model.Write('model')
modelFile.ls()
modelFile.Close()

# Now we perform our sPlot training as before using the externTeacher.
# We do not specify the target, instead we pass the name of the modelFile
# and the discriminating variable.
import subprocess
subprocess.call("externTeacher --workingDirectory TMVA --prefix SPlot --methodName FastBDT --methodType Plugin "
                "--methodConfig '!H:!V:CreateMVAPdfs:NbinsMVAPdf=100:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:"
                "NTreeLayers=3' "
                "--variables 'daughter(0, Kid)' 'daughter(1, piid)' 'daughter(0, chiProb)' 'daughter(1, chiProb)' "
                "'useRestFrame(daughter(0, p))' 'useRestFrame(daughter(1, p))' 'dr' 'dz' 'chiProb' "
                "--modelFileName SPlotModel.root --discriminatingVariables M", shell=True)

# We create another path, and apply the training on the same data,
# usually one wants to use an independent data set for this step.

main = create_path()
inputMdstList('MC5', filelist, path=main)
fillParticleList('K-', 'Kid > 0.1', path=main)
fillParticleList('pi+', 'piid > 0.1', path=main)

# the vertex fit changes the mass a bit; if we cut later, the bins
# at the edges will have fewer statistics because of that. Thus,
# take a bit more, 0.0025 GeV, on each side, and cut it after the fit.
reconstructDecay('D0:raw -> K- pi+', '1.8275 < M < 1.9025', path=main)
vertexKFit('D0:raw', 0.001, path=main)
# because the vertex fix changes the mass a bit
cutAndCopyList('D0', 'D0:raw', '1.83 < M < 1.9', path=main)
matchMCTruth('D0', path=main)

# In the standard case everything works like in the basics_expert.py example

applyTMVAMethod('D0', prefix='SPlot', method="FastBDT", expertOutputName='SPlotProbability', workingDirectory="TMVA", path=main)
variablesToNTuple('D0', ['extraInfo(SPlotProbability)', 'isSignal'], filename='TMVA/SPlotOutput.root', path=main)

# In the future there will be (hopefully) a more advanced sPlot training
# which allow the usage of correlated variables.
# At the moment its just an idea and some code in the TMVAInterface.
# Just set the signal class to 3 in this case
# applyTMVATraining('D0', prefix='SPlot', method="FastBDT", signalClass=3,
#                   expertOutputName='SPlotProbability', workingDirectory="TMVA", path=main)

process(main)
print(statistics)

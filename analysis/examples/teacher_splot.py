#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Uses TMVATeacher module to train a classifier to do electron identification
# using a combination of available PID variables.
#
# Use 'showTMVAResults TMVA.root' to view detailed detailed information about
# a completed training

import sys
import os
import os.path
from basf2 import *
from modularAnalysis import *
import ROOT
from ROOT import (RooRealVar, RooGaussian, RooChebychev, RooAddPdf, RooArgList, RooFit, RooAbsReal, TFile, kFALSE)

# configuration
splotPrefix = "TMVASPlot"
modelFileName = splotPrefix + "_model.root"
modelObjectName = "model"


main = create_path()

main.add_module(register_module('RootInput'))
main.add_module(register_module('Gearbox'))
main.add_module(register_module('ParticleLoader'))

if not os.path.isfile("input.root"):
    # Train TMVA Teacher using all electron candidates as input data
    # Select the candidates
    fillParticleList('K-', 'Kid > 0.1', path=main)
    fillParticleList('pi+', 'piid > 0.1', path=main)

    # the vertex fit changes the mass a bit, if we cut later, the bins
    # at the edges will have fewer statistics because of that. Thus,
    # take a bit more, 0.0025 GeV, on each side, and cut it after the fit.
    reconstructDecay('D0:raw -> K- pi+', '1.8275 < M < 1.9025', path=main)
    vertexKFit('D0:raw', 0.001, path=main)
    # because the vertex fix changes the mass a bit
    cutAndCopyList('D0', 'D0:raw', '1.83 < M < 1.9', path=main, writeOut=True)

    matchMCTruth('D0', path=main)


# Define Variables
variables = ['daughter(0, Kid)', 'daughter(1, piid)',
             'decayAngle(0)',
             'daughter(0, dr)', 'daughter(0, dz)',
             'daughter(1, dr)', 'daughter(1, dz)',
             'daughter(0, chiProb)', 'daughter(1, chiProb)',
             'useCMSFrame(p)', 'useCMSFrame(pt)', 'useCMSFrame(pz)',
             'dr', 'dz', 'chiProb', 'significanceOfDistance',
             'VertexZDist']


discriminatingVariables = ['M']
# Dictionary which defines the PDF for each variable. Currently the PDF is represented
# by a list of float values.
# Define one or multiple methods.
# Every definition consists of 3 string. First the name of the method, secondly the type and thirdly a TMVA config string
methods = [
    ('BDTStochastic',
     'BDT',
     '!H:!V:NTrees=100:BoostType=Grad:Shrinkage=0.10:GradBaggingFraction=0.5:UseBaggedBoost:nCuts=256:MaxDepth=3'),
    ('BDT',
     'BDT',
     '!H:!V:NTrees=100:BoostType=Grad:Shrinkage=0.10:GradBaggingFraction=1.0:nCuts=256:MaxDepth=3'),
    ('FastBDT',
     'Plugin',
     '!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3:sPlot'),
    ('FastBDT2',
     'Plugin',
     '!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3'),
    ('FastBDT3',
     'Plugin',
     '!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=1.0:NCutLevel=8:NTreeLayers=3'),
    ('NeuroBayes',
     'Plugin',
     'H:V:Preprocessing=122:ShapeTreat=OFF:NtrainingIter=20:NBIndiPreproFlagByVarname=daughter0Kid=94,daughter1piid=94,' +
     ",".join(
         ROOT.Belle2.Variable.makeROOTCompatible(variable) +
         "=34" for variable in variables[
             2:]))]


#
# construct PDF
#

# observable
M = RooRealVar("M", "M", 1.83, 1.9)
M.setBins(250)

# Setup component PDFs
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

# Add signal and background
# initial value and maximal value will be set inside TMVASPlotTeacher
bkgfrac = RooRealVar("bkgfrac", "fraction of background", 42, 0., 42)
sigfrac = RooRealVar("sigfrac", "fraction of background", 42, 0., 42)

# Parameters for TMVASPlotTeacher
modelYieldsObjectNames = ['bkgfrac', 'sigfrac']
modelYieldsInitialFractions = [0.99, 0.01]
modelPlotComponentNames = ['sig', 'bkg']

bkgfrac.setConstant(kFALSE)
sigfrac.setConstant(kFALSE)

model = RooAddPdf(modelObjectName, "bkg+sig", RooArgList(bkg, sig), RooArgList(bkgfrac, sigfrac))

# Write model to file and close the file, so TMVASPlotTeacher can open it
modelFile = TFile(modelFileName, "RECREATE")
model.Write(modelObjectName)
modelFile.ls()
modelFile.Close()


# Add TMVA Teacher to path
splotteacher = register_module('TMVASPlotTeacher')
splotteacher.param('prefix', splotPrefix)
splotteacher.param('methods', methods)
splotteacher.param('variables', variables)
splotteacher.param('spectators', discriminatingVariables)
splotteacher.param('discriminatingVariables', discriminatingVariables)
splotteacher.param('modelFileName', modelFileName)
splotteacher.param('modelObjectName', modelObjectName)
splotteacher.param('modelYieldsObjectNames', modelYieldsObjectNames)
splotteacher.param('modelYieldsInitialFractions', modelYieldsInitialFractions)
splotteacher.param('modelPlotComponentNames', modelPlotComponentNames)
splotteacher.param('prepareOption', '!V:SplitMode=alternate:MixMode=Block:NormMode=None')
splotteacher.param('listNames', 'D0')
main.add_module(splotteacher)

teacher = register_module('TMVATeacher')
teacher.param('prefix', 'TMVA')
teacher.param('methods', methods)
teacher.param('variables', variables)
teacher.param('spectators', discriminatingVariables)
teacher.param('target', 'isSignal')
teacher.param('prepareOption', '!V:SplitMode=random:NormMode=None')
teacher.param('listNames', 'D0')
main.add_module(teacher)

cutAndCopyList('D0:background', 'D0', 'isSignal == 0', path=main)

teacher = register_module('TMVATeacher')
teacher.param('prefix', 'TMVAMCorrelation')
teacher.param('methods', methods)
teacher.param('variables', variables)
teacher.param('spectators', discriminatingVariables)
teacher.param('target', 'isInRegion(M, 1.855, 1.875)')
teacher.param('prepareOption', '!V:SplitMode=random:NormMode=None')
teacher.param('listNames', 'D0:background')
main.add_module(teacher)

main.add_module("ProgressBar")

if not os.path.isfile("input.root"):
    main.add_module('RootOutput')

process(main)
print statistics

B2INFO("")
B2INFO("Training completed. Run 'showTMVAresults TMVA_1.root' to view detailed"
       " information about the trained methods.")

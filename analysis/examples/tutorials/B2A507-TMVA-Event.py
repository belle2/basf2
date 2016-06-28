#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to perform an event-based training.
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

teacher = register_module('TMVAOnTheFlyTeacher')
# Takes the same arguments as the TMVATeacher module
teacher.param('workingDirectory', 'TMVA')
# Takes the same arguments as the TMVATeacher module
teacher.param('prefix', 'Event')
# Keep in mind that you can only use variables from the event-based section of the VariableManager!
teacher.param('variables', ['nTracks', 'nECLClusters', 'nKLMClusters', 'ECLEnergy'])
# This training shall identify if the energy in the klm is below 0.1 GeV, a more reasonable training
# would be in the variable isContinuumEvent, however in this case you would need special continuum MC
teacher.param('target', 'passesCut(KLMEnergy < 0.1)')
# In addition we normalise, decorrelate and transform the input variables to a gaussian using the TMVA option VarTransform,
# there are a lot more possibilities, e.g. you can restrict the transformations to a subset of variables
teacher.param(
    'methods', [
        ('FastBDT', 'Plugin',
         '!H:!V:VarTransform=N,D,G:CreateMVAPdfs:NbinsMVAPdf=100:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:'
         'NTreeLayers=3')])
main.add_module(teacher)

process(main)
print(statistics)

# We create another path, and apply the training on the same data,
# usually one wants to use an independent data set for this step.

main = create_path()
inputMdstList('MC5', filelist, path=main)

# Write Out the necessary data for an electron identification training
expert = register_module('TMVAExpert')
expert.param('workingDirectory', 'TMVA')
expert.param('prefix', 'Event')
expert.param('method', 'FastBDT')
expert.param('expertOutputName', 'SignalProbability')
# We apply the expert to the whole event, by leaving the listNames empty (one could just ignore this parameter)
main.add_module(expert)

ntuple = register_module('VariablesToNtuple')
ntuple.param('fileName', 'TMVA/EventOutput.root')
ntuple.param('variables', ['extraInfo(SignalProbability)', 'passesCut(KLMEnergy < 0.1)'])
# Again, we leave the particleList empty!
main.add_module(ntuple)

process(main)
print(statistics)

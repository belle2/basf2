#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to perform a multiclass training.
# There are 5 different classes we want to separate in the FS.
#   Pions
#   Kaons
#   Muons
#   Electrons
#   Protons
#
# If the target variable of the TMVA training has more than 2 distinct values,
# a multiclass classification with a 1 vs rest classification is done automatically.
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
fillParticleList('e-', '', False, path=main)

# Write Out the necessary data for an electron identification training
teacher = register_module('TMVATeacher')
teacher.param('workingDirectory', 'TMVA')
teacher.param('prefix', 'Multiclass')
# The variables which should be used in the training, must be available via the VariableManager
teacher.param('variables', ['eid', 'muid', 'piid', 'prid', 'Kid', 'useCMSFrame(p)', 'chiProb'])
# Additional spectator variables, we include the absolute value of the mcTruth PDG code of the candidate
teacher.param('spectators', ['abs(mcPDG)'])
teacher.param('listNames', 'e-')
main.add_module(teacher)

process(main)
print(statistics)

# The extern teachers automatically trains each class against the rest if there are more than 2 clusters
# identified in the target variable. The different trainings can be distinguished by the signalClass-id
# (which is added as a suffix to all created files)
# The TMVAExpert module will use the correct training if the correct signalClass id is passed as an argument.
import subprocess
subprocess.call("externTeacher --workingDirectory TMVA --prefix Multiclass --methodName FastBDT --methodType Plugin "
                "--methodConfig '!H:!V:CreateMVAPdfs:NbinsMVAPdf=100:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:"
                "NTreeLayers=3' --target 'abs(mcPDG)' --variables eid muid Kid piid prid 'useCMSFrame(p)' chiProb", shell=True)

main = create_path()
inputMdstList('MC5', filelist, path=main)
fillParticleList('e-', '', False, path=main)

# In a multiclass problem the signal class is the target value for the class which should be identified
applyTMVAMethod('e-', prefix='Multiclass', signalClass=11, expertOutputName='ElectronProbability',
                workingDirectory='TMVA', path=main)
applyTMVAMethod('e-', prefix='Multiclass', signalClass=211, expertOutputName='PionProbability', workingDirectory='TMVA', path=main)
variablesToNTuple('e-', ['extraInfo(ElectronProbability)', 'extraInfo(PionProbability)', 'abs(mcPDG)'],
                  filename='TMVA/MulticlassOutput.root', path=main)

process(main)
print(statistics)

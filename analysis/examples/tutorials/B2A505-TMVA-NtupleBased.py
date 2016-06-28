#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to use the extern Teacher on data produced
# by the VariablesToNtuple module and NtupleTools module.
# The inverseSamplingRate and onTheFly feature is obviously not available,
# and there are other obstacles.
#
# But if you already have the data in the corresponding format, it sometimes
# is easier to use the output of the NtupleTools directly instead of
# writing out everything again using the TMVATeacher.
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

# Select all electron candidates
fillParticleList('e-', '', False, path=main)

ntuple = register_module('VariablesToNtuple')
# The name of the file (without the extension) is the prefix you have to use for the externTeacher
ntuple.param('fileName', 'TMVA/VariablesToNtuple.root')
# Make sure the tree name has the form *_tree, otherwise the externTeacher won't find the tree automatically
ntuple.param('treeName', 'SOMETHING_tree')
# Write out all variables you need later for the training
ntuple.param('variables', ['eid', 'useCMSFrame(p)', 'chiProb', 'isSignal'])
ntuple.param('particleList', 'e-')
main.add_module(ntuple)

# Now we use the NtupleTools to write out information about the electrons, the problem is that the resulting branches
# do not correspond to anything in the VariableManager, therefore we cannot simply apply the resulting training
# using the TMVAExpert, because we do not now howto extract the used features from the candidate.
# This can be solved by setting aliases for the branchNames to the correct Variables in the VariableManager by hand
tools = ['EventMetaData', 'e-']
tools += ['RecoStats', 'e-']
tools += ['Kinematics', '^e-']
tools += ['Track', '^e-']
tools += ['PID', '^e-']
tools += ['MCTruth', '^e-']
tools += ['MCKinematics', '^e-']
tools += ['MCHierarchy', '^e-']

# The name of the file (without the extension) is the prefix you have to use for the externTeacher
ntupleFile('TMVA/NtupleTools.root', path=main)
# Make sure the tree name has the form *_tree, otherwise the externTeacher won't find the tree automatically
# So if you're tree is named differently rename it or set an alias!
ntupleTree('elec_tree', 'e-', tools, path=main)

process(main)
print(statistics)

# Now we use the created sample stored in the root file to do a TMVA training.
# You will get a warning due to a missing __weight__ branch, this branch is automatically added.
import subprocess
subprocess.call("externTeacher --workingDirectory TMVA --prefix VariablesToNtuple --methodName FastBDT --methodType Plugin "
                "--methodConfig '!H:!V:CreateMVAPdfs:NbinsMVAPdf=100:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:"
                "NTreeLayers=3' --target isSignal --variables eid 'useCMSFrame(p)' chiProb", shell=True)

# For the ntuple file there is no feature which can simply be used as target (so a feature which returns 0 or 1).
# So we need to add one by hand.
import array
import ROOT

file = ROOT.TFile("TMVA/NtupleTools.root", "UPDATE")
original_tree = file.Get('elec_tree')
cloned_tree = original_tree.CloneTree(0)

target = array.array("f", [0.0])
branch = cloned_tree.Branch("isSignal", target, "isSignal/F")

for i in range(original_tree.GetEntries()):
    original_tree.GetEntry(i, 1)
    mcPDG = original_tree.GetLeaf('e_mcPDG').GetValue()
    target[0] = float(abs(mcPDG) == 11)
    cloned_tree.Fill()

cloned_tree.Write()
file.Write()
file.Close()

# You will get a warning due to a missing __weight__ branch, this branch is automatically added.
# In addition you will get warnings that the branchNames are not registered as variables in the VariableManager,
# this is fine for the training. As mentioned before you must provide the correct aliases to apply this training later.
# Or rename all the used branches accordingly (however this is shouldn't be done by a layman).
subprocess.call("externTeacher --workingDirectory TMVA --prefix NtupleTools --methodName FastBDT --methodType Plugin "
                "--methodConfig '!H:!V:CreateMVAPdfs:NbinsMVAPdf=100:NTrees=400:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:"
                "NTreeLayers=3' --target isSignal --variables e_PIDe e_P e_TrPval", shell=True)


main = create_path()
inputMdstList('MC5', filelist, path=main)
fillParticleList('e-', '', False, path=main)

# For the VariablesToNtuple based-training the application is the same
# as described in basics_expert.py
applyTMVAMethod('e-', prefix='VariablesToNtuple', method="FastBDT", expertOutputName='VariablesToNtupleProbability',
                workingDirectory="TMVA", path=main)

# For the NtupleTools based-training it's a little bit more complicated,
# since we need to set the correct aliases for each feature used in the training first:
from variables import variables
variables.addAlias('e_PIDe', 'eid')
variables.addAlias('e_P', 'p')
variables.addAlias('e_TrPval', 'chiProb')

# Now we can use the training as usual
applyTMVAMethod('e-', prefix='NtupleTools', method="FastBDT", expertOutputName='NtupleToolsProbability',
                workingDirectory="TMVA", path=main)

# Write out signal probability and mc truth
variablesToNTuple('e-', ['extraInfo(VariablesToNtupleProbability)', 'extraInfo(NtupleToolsProbability)', 'isSignal'],
                  filename='TMVA/NtupleOutput.root', path=main)

process(main)
print(statistics)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to perform a simple
# TMVA training on electron candidates using the following
# input variables:
#  - electron pid information
#  - total momentum in the CMS frame
#  - p-value of the track fit
# The standard TMVA method FastBDT is used to perform the training.
#
# First the necessary training data is written to a file using the TMVATeacher module
# Secondly a TMVA training is performed using the externTeacher tool
# Lastly both trainings are applied using the TMVAExpert module
#
# TMVA offers a lot of possibilities, there is a TMVAUserGuide which describes covers the possibilities.
# The TMVAInterface of basf2 can only perform classification tasks until know, but we could implement Regression
# as well if someone needs it. You can inspect all above results using the showTMVAResults tool
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

# The TMVATeacher module writes out the necessary data
# for an electron identification training
teacher = register_module('TMVATeacher')
# Set the working directory, where the data, the config and weight files are stored
teacher.param('workingDirectory', 'TMVA')
# The prefix is used for the root file containing the data, and later for the weight and config files
teacher.param('prefix', 'Basic')
# The variables which should be used in the training, must be available via the VariableManager
teacher.param('variables', ['eid', 'useCMSFrame(p)', 'chiProb'])
# Additional spectator variables, like the target, weights or discriminating variables for sPlot
teacher.param('spectators', ['isSignal'])
# One or more particle lists, for each candidate the above variables are calculated and stored as a sample
teacher.param('listNames', 'e-')
main.add_module(teacher)

process(main)
print(statistics)

# Now we use the created sample stored in the output/Basic.root file to do a TMVA training.
# We pass the configuration for method we want to use, the target variable, and the feature variables.
import subprocess
subprocess.call("externTeacher --workingDirectory TMVA --prefix Basic --methodName FastBDT --methodType Plugin "
                "--methodConfig '!H:!V:CreateMVAPdfs:NbinsMVAPdf=100:NTrees=400:Shrinkage=0.10:RandRatio=0.5:"
                "NCutLevel=8:NTreeLayers=3' --target isSignal --variables eid 'useCMSFrame(p)' chiProb", shell=True)

# We create another path, and apply the training on the same data,
# usually one wants to use an independent data set for this step.

main = create_path()
inputMdstList('MC5', filelist, path=main)
fillParticleList('e-', '', False, path=main)

# Apply the TMVA training using the TMVAExpert module
expert = register_module('TMVAExpert')
# Set the correct working directory
expert.param('workingDirectory', 'TMVA')
# The prefix is used to find the config and weight files
expert.param('prefix', 'Basic')
# Multiple methods can be stored under a given prefix, therefore we have to set the correct name of the method
expert.param('method', 'FastBDT')
# The output of the TMVA method is stored in the extra info field of each particle under the name SignalProbability
expert.param('expertOutputName', 'SignalProbability')
# One or more particle lists, for each candidate the above variables are calculated and stored as a sample
expert.param('listNames', 'e-')
main.add_module(expert)

# Write out signal probability and mc truth
variablesToNTuple('e-', ['extraInfo(SignalProbability)', 'isSignal'], filename='TMVA/BasicOutput.root', path=main)

process(main)
print(statistics)

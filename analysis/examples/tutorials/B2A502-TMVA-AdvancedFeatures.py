#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# This file describes the inverseSamplingRate feature
# and howto normalise event weights using TMVA
#
#######################################################
#
# This tutorial demonstrates how to perform a more advanced
# TMVA training on electron candidates:
#   - Use sampling to reduce the amount of stored background candidates without changing the signal fraction
#   - Set an upper boundary for the maximum amount of candidates
#   - Use TMVA's NormMode to normalise the signal and background weights
#
# The standard TMVA method FastBDT is used to perform the training.
#
# Afterwards the training is applied using the TMVAExpert, some advanced features are shown
#   - Transform the output of a classifier to a probability (which is done as default anyway)
#   - Set another signal fraction which differs from the training sample
#   - Set the signal class id, which is necessary if the signal class is not 1
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

# There are a lot of background candidates in this training. This can use up a lot of storage space, without improving the training!
# Therefore one wants to increase the signal fraction by taking only every n-th background candidate.
teacher = register_module('TMVATeacher')
main.add_module(teacher)
teacher.param('workingDirectory', 'TMVA')
teacher.param('prefix', 'Sampling')
teacher.param('variables', ['eid', 'useCMSFrame(p)', 'chiProb'])
teacher.param('listNames', 'e-')
# We use the target variable isSignal as sampling variable
teacher.param('sample', 'isSignal')
# And whenever the sample variable returns 0, we take only every 5th candidate,
# the weight of each background candidate will be 5,
# so this doesn't change the actual signalFraction, only the amount of
# data which has to be stored. The normalisation of the events is handled
# by TMVA (see below).
teacher.param('inverseSamplingRates', {0: 5})
# We set an upper boundary for the maximum amount of samples stored, you only want to set this in some special cases,
# since this is dangerous if you process different kind of data-files (e.g. first 100M events are Y4S, after that continuum
# -> you miss candidates from the continuum!).
teacher.param('maxSamples', int(1e6))

process(main)
print(statistics)

# Now we train the sampled data.
# In addition we pass an configuration string which is given to TMVA's Factory::PrepareTrainingAndTestTree -- the prepareOption
# SplitMode defines how the data is split into training and test data
# (random in this case, other possibilities are alternate and block)
# MixMode defines how the data is presented to the TMVA method (also random in this case)
# NormMode defines how the weights are normalised (EqualNumEvents means
# the average signal weight is 1 AND SumSignalEvents == SumBackgroundEvents)
import subprocess
subprocess.call("externTeacher --workingDirectory TMVA --prefix Sampling --methodName FastBDT --methodType Plugin "
                "--methodConfig '!H:!V:CreateMVAPdfs:NbinsMVAPdf=100:NTrees=400:Shrinkage=0.10:RandRatio=0.5:"
                "NCutLevel=8:NTreeLayers=3' --target isSignal --variables eid 'useCMSFrame(p)' chiProb "
                "--prepareOption '!V:SplitMode=random:MixMode=SameAsSplitMode:NormMode=EqualNumEvents'", shell=True)

# TMVA offers a lot of possibilities, there is a TMVAUserGuide which describes covers the possibilities.
# The TMVAInterface of basf2 can only perform classification tasks until know, but we could implement
# Regression as well if someone needs it.
# You can inspect all above results using the showTMVAResults tool

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
# The signal class is the highest value of the target variable used in the
# training (usually 1 for signal, which is the default value)
expert.param('signalClass', 1)
# The output of the TMVA method is stored in the extra info field of each particle under the name SignalProbability
expert.param('expertOutputName', 'SignalProbability')
# To ensure that the output is a probability, TMVA can automatically convert the classifier output to a probability
# The default value for this parameter is True.
# Be careful: You must include the option 'CreateMVAPdfs' in the training
# AND you want also to set 'NbinsMVAPdf=100' or to another reasonable value (50-400), TMVA default value is too high sqrt(NSamples)!
expert.param('transformToProbability', True)
# The signal fraction which is used to transform the output can also be
# set (default -1 means the signal fraction of the training sample is
# used). In this case we say that the signal fraction is 0.5 on the sample
# we apply the Expert on (which is wrong, just as a demonstration).
expert.param('signalFraction', 0.01)
# One or more particle lists, for each candidate the above variables are calculated and stored as a sample
expert.param('listNames', 'e-')
main.add_module(expert)

# Write out signal probability and mc truth
variablesToNTuple('e-', ['extraInfo(SignalProbability)', 'isSignal'], filename='TMVA/SamplingOutput.root', path=main)

process(main)
print(statistics)

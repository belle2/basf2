#!/usr/bin/env python3

# Doxygen should skip this script
# @cond

"""
An example script to find a specific decay chain at MC level.
"""

__authors__ = "Yo Sato"

import basf2
from ROOT import Belle2
from modularAnalysis import inputMdst, fillParticleListFromMC, variablesToNtuple, newFindMCDecay
from variables.utils import create_aliases_for_selected
from variables import variables as vm  # shorthand for the variable manager instance

basf2.set_log_level(basf2.LogLevel.DEBUG)

mypath = basf2.create_path()
testinput = Belle2.FileSystem.findFile('analysis/tests/mdst.root')
inputMdst("default", testinput, path=mypath)

fillParticleListFromMC('K+:MC', 'mcPrimary', path=mypath)
fillParticleListFromMC('pi+:MC', 'mcPrimary', path=mypath)
fillParticleListFromMC('e+:MC', 'mcPrimary', path=mypath)
fillParticleListFromMC('nu_e:MC', 'mcPrimary', path=mypath)
fillParticleListFromMC('gamma:MC', 'mcPrimary', path=mypath)

newFindMCDecay('pi0:gg =direct=> gamma:MC gamma:MC', 'isSignal==1', path=mypath)
newFindMCDecay(
    'B+:DstENu =direct=> [anti-D*0 =direct=> [anti-D0 =direct=> K+:MC pi-:MC pi0:gg] pi0:gg ] e+:MC nu_e:MC ',
    'isSignal==1',
    path=mypath)

# One can directly reconstruct pi0:gg in same decay string as following.
# But it this case, you have to use different label for pi0, pi0:gg and pi0:gg2. Otherwise you will have events overlapping.
# newFindMCDecay(
#     'B+:DstENu =direct=>\
# [anti-D*0 =direct=> [anti-D0 =direct=> K+:MC pi-:MC [pi0:gg =direct=> gamma:MC gamma:MC]] [pi0:gg2 =direct=> gamma:MC gamma:MC] ]\
# e+:MC nu_e:MC ',
#     'isSignal==1',
#     path=mypath)


interesting_variables = ['isSignal', "Mbc", "deltaE", "isSignal", "mcErrors"]

vm.printAliases()

variablesToNtuple("B+:DstENu", interesting_variables, path=mypath)

basf2.process(mypath)
print(basf2.statistics)

# @endcond

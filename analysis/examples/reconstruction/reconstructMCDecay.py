#!/usr/bin/env python3

# Doxygen should skip this script
# @cond

"""
An example script to find a specific decay chain at MC level.
"""

__authors__ = "Yo Sato"

import basf2
from modularAnalysis import fillParticleListFromMC, inputMdst, reconstructMCDecay, variablesToNtuple
from variables import variables as vm  # shorthand for the variable manager instance

basf2.set_log_level(basf2.LogLevel.DEBUG)

mypath = basf2.create_path()
testinput = basf2.find_file('analysis/tests/mdst.root')
inputMdst("default", testinput, path=mypath)

fillParticleListFromMC('K+:MC', 'mcPrimary', path=mypath)
fillParticleListFromMC('pi+:MC', 'mcPrimary', path=mypath)
fillParticleListFromMC('e+:MC', 'mcPrimary', path=mypath)
fillParticleListFromMC('nu_e:MC', 'mcPrimary', path=mypath)
fillParticleListFromMC('gamma:MC', 'mcPrimary', path=mypath)

reconstructMCDecay('pi0:gg =direct=> gamma:MC gamma:MC', '', path=mypath)
reconstructMCDecay(
    'B+:DstENu =direct=> [anti-D*0 =direct=> [anti-D0 =direct=> K+:MC pi-:MC pi0:gg] pi0:gg ] e+:MC nu_e:MC ',
    '',
    path=mypath)


# One can directly reconstruct pi0:gg in same decay string as following.
# But in this case, one has to write sub-decay only once. Otherwise same particles are registered twice.
#
# reconstructMCDecay(
#     'B+:DstENu =direct=>\
# [anti-D*0 =direct=> [anti-D0 =direct=> K+:MC pi-:MC [pi0:gg =direct=> gamma:MC gamma:MC]] pi0:gg ]\
# e+:MC nu_e:MC ',
#     '',
#     path=mypath)


interesting_variables = ['isSignal', "Mbc", "deltaE", "mcErrors"]

vm.printAliases()

variablesToNtuple("B+:DstENu", interesting_variables, path=mypath)

basf2.process(mypath)
print(basf2.statistics)

# @endcond

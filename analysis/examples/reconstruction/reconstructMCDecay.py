#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Doxygen should skip this script
# @cond

"""
An example script to find a specific decay chain at MC level.
"""

import basf2
from modularAnalysis import fillParticleListFromMC, inputMdst, reconstructMCDecay, variablesToNtuple
from variables import variables as vm  # shorthand for the variable manager instance

basf2.set_log_level(basf2.LogLevel.DEBUG)

mypath = basf2.create_path()
testinput = basf2.find_file('analysis/tests/mdst.root')
inputMdst("default", testinput, path=mypath)

fillParticleListFromMC('K+:primaryMC', 'mcPrimary', path=mypath)
fillParticleListFromMC('pi+:primaryMC', 'mcPrimary', path=mypath)
fillParticleListFromMC('e+:primaryMC', 'mcPrimary', path=mypath)
fillParticleListFromMC('nu_e:primaryMC', 'mcPrimary', path=mypath)
fillParticleListFromMC('gamma:primaryMC', 'mcPrimary', path=mypath)

reconstructMCDecay('pi0:gg =direct=> gamma:primaryMC gamma:primaryMC', '', path=mypath)
reconstructMCDecay(
    'B+:DstENu =direct=> [anti-D*0 =direct=> [anti-D0 =direct=> K+:primaryMC pi-:primaryMC pi0:gg] pi0:gg ]\
     e+:primaryMC nu_e:primaryMC ',
    '',
    path=mypath)


# One can directly reconstruct pi0:gg in same decay string as following.
# But in this case, one has to write sub-decay only once. Otherwise same particles are registered twice.
#
# reconstructMCDecay(
#     'B+:DstENu =direct=>\
# [anti-D*0 =direct=> [anti-D0 =direct=> K+:primaryMC pi-:primaryMC [pi0:gg =direct=> gamma:primaryMC gamma:primaryMC]] pi0:gg ]\
# e+:primaryMC nu_e:primaryMC ',
#     '',
#     path=mypath)


interesting_variables = ['isSignal', "Mbc", "deltaE", "mcErrors"]

vm.printAliases()

variablesToNtuple("B+:DstENu", interesting_variables, path=mypath)

basf2.process(mypath)
print(basf2.statistics)

# @endcond

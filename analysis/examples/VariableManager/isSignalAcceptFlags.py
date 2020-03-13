#!/usr/bin/env/python3
# -*-coding: utf-8-*-

# add_isSignalAcceptFlags() in variables.utils decodes the mcErrors codes into human readable tags
# like `isSignalAcceptMissingGamma`. After release-05-00-00 all isSignalSomething tags would be
# implemented in add_isSignalAcceptFlags() instead of analysis/variables/src/MCTruthVariables.cc.
#
# Example:
#
#     Call this function before using isSignalSomething:
#     >>> variables.utils.add_isSignalAcceptFlags()
#     >>> variables.variables.addAlias("D0_isSignalAcceptMissingGamma", "daughter(0, isSignalAcceptMissingGamma)")
#     >>> modularAnalysis.variablesToNtuple("D*+:reconstruction",
#     ...                                   ["isSignalAcceptMissingGamma", "D0_isSignalAcceptMissingGamma"],
#     ...                                   filename=YourFile, treename=YourTreeName, path=YourPath)
#
# The decoding process is implemented by the meta function unmask(), which looks like:
# >>> vm.addAlias("isSignalAcceptMissingGammaAndDecayInFlight", "passesCut(unmask(mcErrors," +
# ...             "%d) == %d)" % (c_MissGamma | c_DecayInFlight, c_Correct))
# For more infomation, please check analysis/scripts/variables/utils.py. And you could also easily
# learn how to define a new isSignalSomething from the code in add_isSignalAcceptFlags().
#
# Guanda Gong
#
# For full documentation please refer to https://software.belle2.org
# Anything unclear? Ask questions at https://questions.belle2.org

import basf2 as b2
import modularAnalysis as ma
from variables import variables as vm
import variables.collections as vc
import variables.utils as vu
import stdV0s

# adjust log level, which would be helpful when debugging
# b2.logging.log_level = b2.LogLevel.DEBUG
b2.logging.log_level = b2.LogLevel.INFO

# Create a new path
mypath = b2.Path()

# Add input file and ParticleLoader modules to the path
ma.inputMdstList('default', [b2.find_file('analysis/tests/mdst.root')], path=mypath)

# Add ParticleLoader and ParticleCombiner modules to the path, reconstructing the process
# Ds*+ -> pi+ D0
# D0 -> pi- pi+ K_S0
# K_S0 -> pi+ pi-
ma.fillParticleList('pi+:all', '', path=mypath)
ma.fillParticleList('K+:all', '', path=mypath)
stdV0s.stdKshorts(path=mypath)
ma.reconstructDecay('D0:Example -> pi-:all pi+:all K_S0:merged', '1.7 < M < 2.0', path=mypath)
ma.reconstructDecay('D*+:Example -> pi+:all D0:Example', '0 < Q < 0.022 and useCMSFrame(p)>2.3', path=mypath)

# add MC matching process, during which the isSignal and mcErrors are calculated
ma.matchMCTruth(list_name='D*+:Example', path=mypath)

# Call add_isSignalAcceptFlags() before using isSignalSomething
vu.add_isSignalAcceptFlags()

# declare what variables are needed
basic_vars = vc.inv_mass + vc.kinematics + vc.mc_truth + vc.mc_variables

isSignalSomething_vars = ["isSignalAcceptWrongFSPs"]
isSignalSomething_vars += ["isSignalAcceptMissingNeutrino", "isSignalAcceptMissingMassive"]
isSignalSomething_vars += ["isSignalAcceptMissingGamma", "isSignalAcceptMissing", "mcParticleStatus"]
isSignalSomething_vars += ["isSignalAcceptMissingNeutrinoAndWrongFSP", "isSignalAcceptMissingGammaAndDecayInFlight"]

basic_vars += isSignalSomething_vars


# declare variables of which particles are needed to read out, while declaring the specified variables for
# certain particles like Q for Dst and charge for slow pion
Dst_string_variables = vu.create_aliases_for_selected(list_of_variables=basic_vars + ['Q'],
                                                      decay_string='^D*+ -> pi+ D0',
                                                      prefix='Dst')
Dst_string_variables += vu.create_aliases_for_selected(list_of_variables=basic_vars + ['charge'],
                                                       decay_string='D*+ -> ^pi+ D0',
                                                       prefix='spi')
Dst_string_variables += vu.create_aliases_for_selected(list_of_variables=basic_vars,
                                                       decay_string='D*+ -> pi+ ^D0',
                                                       prefix='D0')
Dst_string_variables += vu.create_aliases_for_selected(list_of_variables=basic_vars,
                                                       decay_string='D*+ -> pi+ [D0 -> ^pi- pi+ K_S0]',
                                                       prefix='pim')
Dst_string_variables += vu.create_aliases_for_selected(list_of_variables=basic_vars,
                                                       decay_string='D*+ -> pi+ [D0 -> pi- ^pi+ K_S0]',
                                                       prefix='pip')
Dst_string_variables += vu.create_aliases_for_selected(list_of_variables=basic_vars,
                                                       decay_string='D*+ -> pi+ [D0 -> pi- pi+ ^K_S0]',
                                                       prefix='Ks')
Dst_string_variables += vu.create_aliases_for_selected(list_of_variables=basic_vars,
                                                       decay_string='D*+ -> pi+ [D0 -> pi- pi+ [K_S0 -> ^pi+ pi-]]',
                                                       prefix='KsPi1')
Dst_string_variables += vu.create_aliases_for_selected(list_of_variables=basic_vars,
                                                       decay_string='D*+ -> pi+ [D0 -> pi- pi+ [K_S0 -> pi+ ^pi-]]',
                                                       prefix='KsPi2')

vm.printAliases()


# save the variables using variablesToNtuple
ma.variablesToNtuple('D*+:Example', Dst_string_variables + isSignalSomething_vars, filename="isSignalAcceptFlags.root",
                     treename='Dst', path=mypath)

b2.process(mypath)
print(b2.statistics)

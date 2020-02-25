#!/usr/bin/env/python3
# -*-coding: utf-8-*-

# The isSignalAcceptFlags(flag1, flag2...) is same as isSignal but would also
# return ture when indicated error flags are set. For example, you could use
# isSignalAcceptFlags(c_MissNeutrino ,c_MisID, c_AddedWrongParticle) to represent
# isSignalAcceptMissingNeutrinoAndWrongFSP:
#   >>>vm.addAlias("isSignalAcceptMissingNeutrinoAndWrongFSP", "isSignalAcceptFlags(c_MissNeutrino ,c_MisID, c_AddedWrongParticle)")
# and
# isSignalAcceptFlags(c_MissGamma, c_DecayInFlight)
# for isSignalAcceptMissGammaAndDecayInFlight:
#   >>>vm.addAlias("isSignalAcceptMissGammaAndDecayInFlight", "isSignalAcceptFlags(c_MissGamma, c_DecayInFlight)")
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

# Create a new path
mypath = b2.Path()

# Add input file and ParticleLoader modules to the path
ma.inputMdstList('default', [b2.find_file('analysis/tests/mdst.root')], path=mypath)

# Or you could use the following file, if you are in kekcc. In this file you would
# find more signal events.
# ma.inputMdst(
#     'default',
#     '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007426/s00/e1003/4S' +
#     '/r00000/ccbar/mdst/sub00/mdst_000001_prod00007426_task10020000001.root',
#     path=mypath)

# Add ParticleLoader and ParticleCombiner modules to the path, reconstructing the process
# Ds*+ -> pi+ D0
# D0 -> pi- pi+ K_S0
# K_S0 -> pi+ pi-
ma.fillParticleList('pi+:all', '', path=mypath)
ma.fillParticleList('K+:all', '', path=mypath)
ma.fillParticleList('K_S0:V0 -> pi+ pi-', '0.3 < M < 0.7', True, path=mypath)
ma.reconstructDecay('K_S0:RD -> pi+:all pi-:all', '0.3 < M < 0.7', 1, True, path=mypath)
ma.copyLists('K_S0:merged', ['K_S0:V0', 'K_S0:RD'], False, path=mypath)
ma.reconstructDecay('D0:Example -> pi-:all pi+:all K_S0:merged', '1.7 < M < 2.0', path=mypath)
ma.reconstructDecay('D*+:Example -> pi+:all D0:Example', '0 < Q < 0.022 and useCMSFrame(p)>2.3', path=mypath)

# add MC matching process, during which the isSignal and mcErrors are calculated
ma.matchMCTruth('K_S0:merged', path=mypath)
ma.matchMCTruth(list_name='D0:Example', path=mypath)
ma.matchMCTruth(list_name='D*+:Example', path=mypath)

# *** Use isSignalAcceptFlags(c_MissNeutrino ,c_MisID, c_AddedWrongParticle) to represent
# isSignalAcceptMissingNeutrinoAndWrongFSP ***
vm.addAlias("isSignalAcceptMissingNeutrinoAndWrongFSP", "isSignalAcceptFlags(c_MissNeutrino ,c_MisID, c_AddedWrongParticle)")
# *** Use isSignalAcceptFlags(c_MissGamma, c_DecayInFlight) to represent isSignalAcceptMissGammaAndDecayInFlight ***
vm.addAlias("isSignalAcceptMissGammaAndDecayInFlight", "isSignalAcceptFlags(c_MissGamma, c_DecayInFlight)")

# declare what variables are needed
basic_vars = vc.inv_mass + vc.kinematics + vc.mc_truth + vc.mc_variables
basic_vars += ["isExtendedSignal", "isSignalAcceptWrongFSPs", "isSignalAcceptBremsPhotons"]
basic_vars += ["isSignalAcceptMissingNeutrino", "isSignalAcceptMissingMassive"]
basic_vars += ["isSignalAcceptMissingGamma", "isSignalAcceptMissing", "mcParticleStatus"]
basic_vars += ["isSignalAcceptMissingNeutrinoAndWrongFSP", "isSignalAcceptMissGammaAndDecayInFlight"]

# declare variables of which particles are needed to read out, while declaring the specified variables for
# certain particles like Q for Dst and charge for slow pion
Dst_string_variables = vu.create_aliases_for_selected(list_of_variables=basic_vars + ['Q'],
                                                      decay_string='^D*+ -> pi+ D0',
                                                      prefix='Dst') + \
                       vu.create_aliases_for_selected(list_of_variables=basic_vars + ['charge'],
                                                      decay_string='D*+ -> ^pi+ D0',
                                                      prefix='spi') + \
                       vu.create_aliases_for_selected(list_of_variables=basic_vars,
                                                      decay_string='D*+ -> pi+ ^D0',
                                                      prefix='D0') + \
                       vu.create_aliases_for_selected(list_of_variables=basic_vars,
                                                      decay_string='D*+ -> pi+ [D0 -> ^pi- pi+ K_S0]',
                                                      prefix='pim') + \
                       vu.create_aliases_for_selected(list_of_variables=basic_vars,
                                                      decay_string='D*+ -> pi+ [D0 -> pi- ^pi+ K_S0]',
                                                      prefix='pip') + \
                       vu.create_aliases_for_selected(list_of_variables=basic_vars,
                                                      decay_string='D*+ -> pi+ [D0 -> pi- pi+ ^K_S0]',
                                                      prefix='Ks') + \
                       vu.create_aliases_for_selected(list_of_variables=basic_vars,
                                                      decay_string='D*+ -> pi+ [D0 -> pi- pi+ [K_S0 -> ^pi+ pi-]]',
                                                      prefix='KsPi1') + \
                       vu.create_aliases_for_selected(list_of_variables=basic_vars,
                                                      decay_string='D*+ -> pi+ [D0 -> pi- pi+ [K_S0 -> pi+ ^pi-]]',
                                                      prefix='KsPi2')

# save the variables using variablesToNtuple
ma.variablesToNtuple('D*+:Example', Dst_string_variables, filename="isSignalAcceptFlags.root",
                     treename='Dst', path=mypath)

b2.process(mypath)
print(b2.statistics)

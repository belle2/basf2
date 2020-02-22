#!/usr/bin/env/python3
# -*-coding: utf-8-*-

import basf2 as b2
import modularAnalysis as ma
from variables import variables as vm
import variables.collections as vc
import variables.utils as vu

# reate a new path
mypath = b2.Path()

# add input file and ParticleLoader modules to the path
ma.inputMdst(
    'default',
    '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007392/s00/e1003/4S' +
    '/r00000/mixed/mdst/sub00/mdst_000141_prod00007392_task10020000141.root',
    path=mypath)

ma.fillParticleList('pi+:all', '', path=mypath)
ma.fillParticleList('K+:all', '', path=mypath)

ma.fillParticleList('K_S0:V0 -> pi+ pi-', '0.3 < M < 0.7', True, path=mypath)
ma.reconstructDecay('K_S0:RD -> pi+:all pi-:all', '0.3 < M < 0.7', 1, True, path=mypath)
ma.copyLists('K_S0:merged', ['K_S0:V0', 'K_S0:RD'], False, path=mypath)
ma.reconstructDecay('D0:Example -> pi-:all pi+:all K_S0:merged', '1.7 < M < 2.0', path=mypath)
ma.reconstructDecay('D*+:Example -> pi+:all D0:Example', '0 < Q < 0.022 and useCMSFrame(p)>2.3', path=mypath)

ma.matchMCTruth('K_S0:merged', path=mypath)
ma.matchMCTruth(list_name='D0:Example', path=mypath)
ma.matchMCTruth(list_name='D*+:Example', path=mypath)

vu.add_isSignalTags()
basic_vars = vc.inv_mass + vc.kinematics + vc.mc_truth + vc.mc_variables \
             + ["isExtendedSignal", "isSignalAcceptWrongFSPs", "isSignalAcceptBremsPhotons",
                "isSignalAcceptMissingNeutrino", "isSignalAcceptMissingMassive",
                "isSignalAcceptMissingGamma", "isSignalAcceptMissing", "mcParticleStatus"]


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

# test new isSignalTags alias expression here
# vm.addAlias("isSignalTest", "passesCut((mcErrors & ~%d) == 0)"%(512))

# ma.variablesToNtuple('D*+:Example', Dst_string_variables + ["isSignalTest"], filename="isSignalTags.root",
ma.variablesToNtuple('D*+:Example', Dst_string_variables, filename="isSignalTags.root",
                     treename='Dst', path=mypath)

b2.process(mypath)
print(b2.statistics)

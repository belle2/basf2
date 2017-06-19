#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# The VariablesToNtuple (and VariablesToTree) module support collections of variables.
# You specify a collection instead of a variable name, and the collection will be automatically
# resolved to a set of variable names
# the Python module variableCollections defines some default collections, just import it


from basf2 import *
from modularAnalysis import *
import variableCollections

# You can also define collections yourself
import variables
variables.variables.addCollection('MyCollection', variables.std_vector('daughter(0, Kid)', 'daughter(1, piid)'))

inputMdstList('MC7', ['/storage/jbod/tkeck/MC7/evtgen-charged/sub00/mdst_000240_prod00000788_task00000685.root'])

fillParticleLists([('K-', 'Kid > 0.2'), ('pi+', 'piid > 0.2')])
reconstructDecay('D0 -> K- pi+', '1.750 < M < 1.95')
matchMCTruth('D0')

# This will write out one row per candidate in the D0 list
analysis_main.add_module('VariablesToNtuple',
                         particleList='D0',
                         variables=['Kinematics', 'MCTruth', 'MyCollection'],
                         fileName='CollectionVariables.root')


process(analysis_main)
print(statistics)

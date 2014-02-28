#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Thomas Keck 2014

from full_reconstruction import *
from basf2 import *

chargedTrackVars = [
    'eid',
    'eid_dEdx',
    'eid_TOP',
    'eid_ARICH',
    'Kid',
    'Kid_dEdx',
    'Kid_TOP',
    'Kid_ARICH',
    'prid',
    'prid_dEdx',
    'prid_TOP',
    'prid_ARICH',
    'muid',
    'muid_dEdx',
    'muid_TOP',
    'muid_ARICH',
    'p',
    'pt',
    'chiProb',
]

DVars = ['prodChildProb', 'p', 'pt', 'p_CMS', 'pt_CMS']
BVars = ['prodChildProb', 'p', 'pt', 'p_CMS', 'pt_CMS']

methods = [
    (
        'BDTGradient_100', 'BDT',
        '!H:CreateMVAPdfs:!V:NTrees=100:BoostType=Grad:Shrinkage=0.10:'
        'UseBaggedGrad:GradBaggingFraction=0.5:nCuts=10:MaxDepth=2'
    ),
    #('NeuroBayes', 'NeuroBayes',
    #'!H:CreateMVAPdfs:V:NTrainingIter=50:TrainingMethod=BFGS')
]
# old syntax, needs to be updated
# 'BDTGradient_1000_20_5': '!H:CreateMVAPdfs:!V:NTrees=1000:BoostType=Grad:
# Shrinkage=0.10:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=20:MaxDepth=5',

fr = FullReconstruction()

# Add FSP
fr.addParticle(Particle('gamma', ['p', 'pt'], methods))
fr.addParticle(Particle('pi0', ['p', 'pt'], methods))
fr.addParticle(Particle('pi+', chargedTrackVars, methods))
fr.addParticle(Particle('K+', chargedTrackVars, methods))

p = Particle('D0', DVars, methods)
p.addChannel(['K-', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['K-', 'pi+', 'pi0'])
fr.addParticle(p)

p = Particle('D+', DVars, methods)
p.addChannel(['K-', 'pi+', 'pi+'])
p.addChannel(['K-', 'K+', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi0'])
p.addChannel(['K-', 'K+', 'pi+', 'pi0'])
fr.addParticle(p)

p = Particle('D*+', DVars, methods)
p.addChannel(['D0', 'pi+'])
p.addChannel(['D+', 'pi0'])
fr.addParticle(p)

p = Particle('D*0', DVars, methods)
p.addChannel(['D0', 'pi0'])
p.addChannel(['D0', 'gamma'])
fr.addParticle(p)

p = Particle('B+', BVars, methods)
p.addChannel(['D0', 'pi+'])
p.addChannel(['D+', 'pi0'])
p.addChannel(['D*-', 'pi+', 'pi+'])
fr.addParticle(p)

p = Particle('B0', BVars, methods)
p.addChannel(['D0', 'pi0'])
p.addChannel(['D-', 'pi+'])
p.addChannel(['D-', 'pi0', 'pi+'])
p.addChannel(['D*-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['D*-', 'pi+', 'pi+', 'pi-', 'pi0'])
fr.addParticle(p)

main = create_path()
main.add_module(register_module('RootInput'))
main.add_module(register_module('ProgressBar'))

main.add_module(register_module('ParticleLoader'))

fr.run(main)

# ntupler = register_module('VariableNtuple')
# ntupler.param('particleList', 'B+')
# ntupler.param('variables', ['p', 'pt', 'M', 'dM', 'Q', 'dQ', 'Mbc', 'deltaE',
# 'nDaughters', 'flavor', 'tmptruth'])
# main.add_module(ntupler)

# show constructed path
print main

process(main)
B2WARNING('event() statistics:')
print statistics
B2WARNING('endRun() statistics:')
print statistics(statistics.END_RUN)

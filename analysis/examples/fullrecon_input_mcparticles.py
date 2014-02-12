#!/usr/bin/env python
# -*- coding: utf-8 -*-

# with a given FR config, run the reconstruction chain,
# but with no cuts, trainings or classification
# Particles are loaded directly from MCParticles,
# so combinatorics should be limited


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

methods = []
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
particleloader = register_module('ParticleLoader')
particleloader.param('UseMCParticles', True)
main.add_module(particleloader)

fr.run(main, mcrun=True)
#after full reconstruction modules (to allow parallel processing)
#main.add_module(register_module('ProgressBar'))

# ntupler = register_module('VariableNtuple')
# ntupler.param('particleList', 'B+')
# ntupler.param('variables', ['p', 'pt', 'M', 'dM', 'Q', 'dQ', 'Mbc', 'deltaE',
# 'nDaughters', 'flavor', 'tmptruth'])
# main.add_module(ntupler)

#printer = register_module('ParticlePrinter')
#printer.param('ListName', 'B+')
#printer.param('FullPrint', False)
#main.add_module(printer)

particlestats = register_module('ParticleStats')
particlestats.param('strParticleLists', ['B+', 'B0', 'D*-', 'D0', 'D-'])
main.add_module(particlestats)

# show constructed path
print main

process(main)
print statistics

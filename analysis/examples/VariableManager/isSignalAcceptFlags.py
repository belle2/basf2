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
# learn how to define a new isSignalSomething from the code in add_isSignalAcceptFlags(). The definition of mc match
# error flags could be found in:
# https://b2-master.belle2.org/software/development/sphinx/analysis/doc/MCMatching.html#error-flags
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

# declare what variables are needed
basic_vars = vc.inv_mass + vc.kinematics + vc.mc_truth + vc.mc_variables

# In principle, add_isSignalAcceptFlags() should be called before using isSignalAcceptSomething. However,
# this function is called automatically in analysis/scripts/variables/__init__.py when importing variables module,
# so you don't need to really call it. All you need to do is be aware of this function is called automatically.
# vu.add_isSignalAcceptFlags()

# The following isSignalSomthing variables are predefined in add_isSignalAcceptFlags(),
isSignalSomething_vars = ["isSignalAcceptWrongFSPs"]
isSignalSomething_vars += ["isSignalAcceptMissingNeutrino", "isSignalAcceptMissingMassive"]
isSignalSomething_vars += ["isSignalAcceptMissingGamma", "isSignalAcceptMissing", "mcParticleStatus"]
isSignalSomething_vars += ["isSignalAcceptMissingNeutrinoAndWrongFSP", "isSignalAcceptMissingGammaAndDecayInFlight"]

# Try to defined your onw isSignalAcceptSomthing

# c_Correct = 0    # This Particle and all its daughters are perfectly reconstructed.
# c_MissFSR = 1    # A Final State Radiation (FSR) photon is not reconstructed (based
# # on MCParticle::c_IsFSRPhoton).
# c_MissingResonance = 2    # The associated MCParticle decay contained additional non-final-state
# # particles (e.g. a rho) that weren't reconstructed. This is probably O.K. in most cases
# c_DecayInFlight = 4    # A Particle was reconstructed from the secondary decay product of the actual particle.
# # This means that a wrong hypothesis was used to reconstruct it, which e.g. for tracks might
# # mean a pion hypothesis was used for a secondary electron.
# c_MissNeutrino = 8    # A neutrino is missing (not reconstructed).
# c_MissGamma = 16   # A photon (not FSR) is missing (not reconstructed).
# c_MissMassiveParticle = 32   # A generated massive FSP is missing (not reconstructed).
# c_MissKlong = 64   # A Klong is missing (not reconstructed).
# c_MisID = 128  # One of the charged final state particles is mis-identified.
# c_AddedWrongParticle = 256  # A non-FSP Particle has wrong PDG code, meaning one of the daughters (or their daughters)
# # belongs to another Particle.
# c_InternalError = 512  # There was an error in MC matching. Not a valid match. Might indicate fake/background track or cluster.
# c_MissPHOTOS = 1024  # A photon created by PHOTOS was not reconstructed (based on MCParticle::c_IsPHOTOSPhoton)
# # A photon added with the bremsstrahlung recovery tools (correctBrems or correctBremsBelle) has no MC particle
# c_AddedRecoBremsPhoton = 2048
# # assigned, or it doesn't belong to the decay chain
vm.addAlias("isSignalAcceptMissingGammaAndMissingNeutrino", "passesCut(unmask(mcErrors, 8, 16) == 0)")
isSignalSomething_vars += ["isSignalAcceptMissingGammaAndMissingNeutrino"]

basic_vars += isSignalSomething_vars

vm.printAliases()


# save the variables using variablesToNtuple
ma.variablesToNtuple('D*+:Example', basic_vars, filename="isSignalAcceptFlags.root",
                     treename='Dst', path=mypath)

b2.process(mypath)
print(b2.statistics)

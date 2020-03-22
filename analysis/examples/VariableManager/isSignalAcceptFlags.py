#!/usr/bin/env/python3
# -*-coding: utf-8-*-

# isSignalAcceptFlags or isSignalAcceptSomething, means a special isSignal variable that accepts certain
# bits in the mcErrors to be set. For example, the pure isSignal equals to 1 only when mcErrors == 0, while
# isSignalAcceptMissingGamma euqals to 1 when mcErrors == 0 or mcErrors == 16, which could be interpreted
# as "After setting the MissGamma bit (the 5th bit, or 16 = 0b00010000) to 0, if mcErrors == 0, then
# isSignalAcceptMissingGamma euqals to 1". The operation, setting certain bits in a variable to 0, is called
# "unmask", in contrast to "mask".
# In a basf2 steering script, new isSignalAcceptSomething variables could be added by unmasking the mcErrors
# code using the meta function unmask() like:
# >>> c_MissGamma = 16
# >>> c_DecayInFlight = 4
# >>> c_Correct = 4
# >>> vm.addAlias("isSignalAcceptMissingGammaAndDecayInFlight", "passesCut(unmask(mcErrors," +
# ...             "%d) == %d)" % (c_MissGamma | c_DecayInFlight, c_Correct))
# The full definition of mc match error flags could be found in:
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

# The following isSignalSomthing variables are predefined in Variable Manager,
isSignalSomething_vars = ["isSignalAcceptWrongFSPs"]
isSignalSomething_vars += ["isSignalAcceptMissingNeutrino", "isSignalAcceptMissingMassive"]
isSignalSomething_vars += ["isSignalAcceptMissingGamma", "isSignalAcceptMissing", "mcParticleStatus"]

# Try to defined your onw isSignalAcceptSomthing
c_Correct = 0    # This Particle and all its daughters are perfectly reconstructed.
c_MissFSR = 1    # A Final State Radiation (FSR) photon is not reconstructed (based
# on MCParticle::c_IsFSRPhoton).
c_MissingResonance = 2    # The associated MCParticle decay contained additional non-final-state
# particles (e.g. a rho) that weren't reconstructed. This is probably O.K. in most cases
c_DecayInFlight = 4    # A Particle was reconstructed from the secondary decay product of the actual particle.
# This means that a wrong hypothesis was used to reconstruct it, which e.g. for tracks might
# mean a pion hypothesis was used for a secondary electron.
c_MissNeutrino = 8    # A neutrino is missing (not reconstructed).
c_MissGamma = 16   # A photon (not FSR) is missing (not reconstructed).
c_MissMassiveParticle = 32   # A generated massive FSP is missing (not reconstructed).
c_MissKlong = 64   # A Klong is missing (not reconstructed).
c_MisID = 128  # One of the charged final state particles is mis-identified.
c_AddedWrongParticle = 256  # A non-FSP Particle has wrong PDG code, meaning one of the daughters (or their daughters)
# belongs to another Particle.
c_InternalError = 512  # There was an error in MC matching. Not a valid match. Might indicate fake/background track or cluster.
c_MissPHOTOS = 1024  # A photon created by PHOTOS was not reconstructed (based on MCParticle::c_IsPHOTOSPhoton)
c_AddedRecoBremsPhoton = 2048   # A photon added with the bremsstrahlung recovery tools (correctBrems or correctBremsBelle)
# has no MC particle assigned, or it doesn't belong to the decay chain

vm.addAlias("isSignalAcceptMissingGammaAndMissingNeutrino", "passesCut(unmask(mcErrors, 8, 16) == 0)")
isSignalSomething_vars += ["isSignalAcceptMissingGammaAndMissingNeutrino"]

vm.addAlias("isSignalAcceptMissingGammaAndDecayInFlight", "passesCut(unmask(mcErrors," +
            "%d) == %d)" % (c_MissGamma | c_DecayInFlight, c_Correct))
isSignalSomething_vars += ["isSignalAcceptMissingGammaAndDecayInFlight"]

vm.addAlias("isSignalAcceptMissingNeutrinoAndWrongFSP", "passesCut(unmask(mcErrors," +
            "%d) == %d)" % (c_MissGamma | c_MissMassiveParticle | c_MissKlong | c_MissKlong, c_Correct))
isSignalSomething_vars += ["isSignalAcceptMissingNeutrinoAndWrongFSP"]

basic_vars += isSignalSomething_vars

vm.printAliases()


# save the variables using variablesToNtuple
ma.variablesToNtuple('D*+:Example', basic_vars, filename="isSignalAcceptFlags.root",
                     treename='Dst', path=mypath)

b2.process(mypath)
print(b2.statistics)

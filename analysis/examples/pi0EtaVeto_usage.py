#!/usr/bin/env python3

# Doxygen should skip this script
# @cond

"""
An example script to use writePi0EtaVeto function to calculate the pi0
and eta probability.
"""

__authors__ = "Yo Sato"

import basf2 as b2
from ROOT import Belle2
import modularAnalysis as ma
from variables import variables as vm  # shorthand for the variable manager instance
from stdCharged import stdK, stdPi
from stdPhotons import stdPhotons

# writePi0EtaVeto uses a payload in analysis global tag.
b2.conditions.prepend_globaltag('analysis_tools_release-04-02')

mypath = b2.create_path()
testinput = Belle2.FileSystem.findFile('analysis/tests/mdst.root')
ma.inputMdst("default", testinput, path=mypath)

stdK("loose", path=mypath)
stdPi("loose", path=mypath)
stdPhotons("loose", path=mypath)

# Example 1 : Calculate pi0/eta probability using gamma
# Reconstruct B0 -> K*0 gamma
ma.reconstructDecay("K*0:Kpi     -> K+:loose pi-:loose", "", path=mypath)
ma.reconstructDecay("B0:Kpigamma -> K*0:Kpi gamma:loose", "useCMSFrame(daughter(1,E))>1.4", path=mypath)
ma.matchMCTruth("B0:Kpigamma", path=mypath)

# writePi0EtaVeto requires to build ROE
ma.buildRestOfEvent("B0:Kpigamma", path=mypath)

# Call writePi0EtaVeto
# particleList : Signal side particle's particleList
# decayString : DecayString specifing a particle which is used to calculate the pi0/eta probability
ma.writePi0EtaVeto(particleList='B0:Kpigamma',
                   decayString='B0 -> [K*0 -> K+ pi-] ^gamma',
                   path=mypath)

# Example 2 : Calculate pi0/eta probability using non-gamma particle.
# Reconstruct B+ -> anti-D0 pi+. This is one of the mode to validate the pi0/eta veto tool.
ma.reconstructDecay("D0:Kpi -> K-:loose pi+:loose", "", path=mypath)
ma.reconstructDecay("B+:Dpi -> anti-D0:Kpi pi+:loose", "useCMSFrame(daughter(1,E))>1.4", path=mypath)
ma.matchMCTruth("B+:Dpi", path=mypath)

# writePi0EtaVeto requires to build ROE
ma.buildRestOfEvent("B+:Dpi", path=mypath)

# Call writePi0EtaVeto
# hardParticle : If you want to use non-gamma particle to calcuate the pi0/eta probability,
#                you have to tell the particle name with an argument hardParticle. (default: gammma)
ma.writePi0EtaVeto('B+:Dpi',
                   'B+ -> [anti-D0 -> K+ pi-] ^pi+',
                   hardParticle='pi+',
                   path=mypath)

vm.addAlias("Pi0ProbOrigin", "extraInfo(Pi0ProbOrigin)")
vm.addAlias("EtaProbOrigin", "extraInfo(EtaProbOrigin)")

interesting_variables = ['isSignal', "Mbc", "deltaE", "Pi0ProbOrigin", "EtaProbOrigin"]

vm.printAliases()

ma.variablesToNtuple("B0:Kpigamma", interesting_variables, treename="Kstgamma", path=mypath)
ma.variablesToNtuple("B+:Dpi",      interesting_variables, treename="Dpi", path=mypath)

b2.process(mypath)
print(b2.statistics)

# @endcond

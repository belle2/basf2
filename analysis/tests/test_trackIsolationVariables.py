##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import pdg
from ROOT import Belle2
import b2test_utils

path = b2.create_path()

b2test_utils.configure_logging_for_tests()
b2.set_random_seed("1337")

ma.inputMdstList(filelist=[b2test_utils.require_file("mdst14.root", "validation")],
                 entrySequences=["0:1"],
                 path=path)

detectors = ["CDC", "TOP", "ARICH", "ECL", "KLM"]

# Pdg code of the charged stable particles & antiparticles.
# Note that this is just to test module behaviour against different user's input:
# in fact, for any choice of particle list's charge, the charge-conjugated one
# gets loaded automatically.

chargedStableList = []
for idx in range(len(Belle2.Const.chargedStableSet)):
    pdgId = Belle2.Const.chargedStableSet.at(idx).getPDGCode()
    chargedStableList.extend([pdgId, -pdgId])

for pname in pdg.to_names(chargedStableList):

    plist = f"{pname}:ref"

    ma.fillParticleList(plist, "[pt > 0.1] and [thetaInCDCAcceptance]", path=path)

    ntup_vars = ma.calculateTrackIsolation(plist,
                                           path,
                                           *detectors)

    ma.printList(plist, full=True, path=path)
    ma.printVariableValues(plist, ntup_vars, path=path)

    ma.variablesToNtuple(plist,
                         ntup_vars,
                         treename=pname,
                         filename="TrackIsolationVariables.root", path=path)

with b2test_utils.clean_working_directory():
    with b2test_utils.set_loglevel(b2.LogLevel.INFO):
        b2.process(path)

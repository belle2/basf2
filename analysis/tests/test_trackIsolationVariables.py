##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import stdV0s as stdv0
import pdg
from ROOT import Belle2
import b2test_utils
import variables.utils as vu

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

for pname, pdgId in zip(pdg.to_names(chargedStableList), chargedStableList):

    plist = f"{pname}:ref"

    ma.fillParticleList(plist, "[pt > 0.1] and [thetaInCDCAcceptance]", path=path)

    track_iso_vars = ma.calculateTrackIsolation(plist,
                                                path,
                                                *detectors,
                                                exclude_pid_det_weights=True)

    ma.printVariableValues(plist, track_iso_vars[pdgId], path=path)
    ma.variablesToNtuple(plist,
                         track_iso_vars[pdgId],
                         treename=pname,
                         filename="TrackIsolationVariables.root",
                         path=path)

v0_types = ["Lambda0:merged", "K_S0:merged"]
v0_functions = [stdv0.stdLambdas, stdv0.stdKshorts]
v0_decays = [" -> ^p+ ^pi-", " -> ^pi+ ^pi-"]

for v0_type, v0_function, v0_decay in zip(v0_types, v0_functions, v0_decays):

    v0_function(path=path)

    track_iso_vars = ma.calculateTrackIsolation(v0_type+v0_decay,
                                                path,
                                                *detectors,
                                                exclude_pid_det_weights=True)

    ntup_vars_aliases = []
    for ivars in track_iso_vars.values():
        ntup_vars_aliases += vu.create_aliases_for_selected(list_of_variables=ivars,
                                                            decay_string=v0_type+v0_decay)

    ma.printVariableValues(v0_type, ntup_vars_aliases, path=path)
    ma.variablesToNtuple(v0_type,
                         ntup_vars_aliases,
                         treename=f"{v0_type.split(':')[0]}",
                         filename="TrackIsolationVariables.root", path=path)

with b2test_utils.clean_working_directory():
    with b2test_utils.set_loglevel(b2.LogLevel.INFO):
        b2.process(path)

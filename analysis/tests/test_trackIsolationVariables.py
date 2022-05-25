##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2 as b2
import modularAnalysis as ma
import stdCharged as stdc
import stdV0s as stdv0
import b2test_utils
import variables.utils as vu

path = b2.create_path()

b2test_utils.configure_logging_for_tests()
b2.set_random_seed("1337")

ma.inputMdstList(filelist=[b2test_utils.require_file("mdst14.root", "validation")],
                 entrySequences=["0:1"],
                 path=path)

ntup_vars = []
detector_tuple = ("CDC", "PID", "ECL", "KLM")
for det in detector_tuple:
    ntup_vars.extend([f"dist3DToClosestTrkAtSurface{det}", f"dist2DRhoPhiToClosestTrkAtSurface{det}"])

for ptype in stdc._chargednames:

    # Note that this is just to test module behaviour against different user's input:
    # in fact, for any choice of particle list's charge, the charge-conjugated one
    # gets loaded automatically.
    for ch in ("+", "-"):

        # Protons are a bit special...
        if ptype == "p" and ch == "-":
            ptype = f"anti-{ptype}"

        pname = f"{ptype}{ch}:all"
        ma.fillParticleList(pname, "", path=path)

        ma.calculateTrackIsolation(pname,
                                   path,
                                   *detector_tuple,
                                   alias="dist3DToClosestTrkAtSurface")
        ma.calculateTrackIsolation(pname,
                                   path,
                                   *detector_tuple,
                                   use2DRhoPhiDist=True,
                                   alias="dist2DRhoPhiToClosestTrkAtSurface")

        ma.printList(pname, full=True, path=path)
        ma.printVariableValues(pname, ntup_vars, path=path)

        ma.variablesToNtuple(pname,
                             ntup_vars,
                             treename=f"{ptype}{ch}",
                             filename="TrackIsolationVariables.root", path=path)
v0_types = ['Lambda0:merged', 'K_S0:merged']
v0_functions = [stdv0.stdLambdas, stdv0.stdKshorts]
v0_decays = [' -> ^p+ ^pi-', ' -> ^pi+ ^pi-']

for v0_type, v0_function, v0_decays in zip(v0_types, v0_functions, v0_decays):
    v0_function(path=path)
    track_vars = vu.create_aliases_for_selected(list_of_variables=ntup_vars,
                                                decay_string=v0_type+v0_decays)
    ma.calculateTrackIsolation(v0_type+v0_decays,
                               path,
                               *detector_tuple,
                               alias="dist3DToClosestTrkAtSurface")
    ma.calculateTrackIsolation(v0_type+v0_decays,
                               path,
                               *detector_tuple,
                               use2DRhoPhiDist=True,
                               alias="dist2DRhoPhiToClosestTrkAtSurface")
    ma.printList(v0_type, full=True, path=path)
    ma.printVariableValues(v0_type, track_vars, path=path)
    ma.variablesToNtuple(v0_type,
                         track_vars,
                         treename=f"{v0_type.split(':')[0]}",
                         filename="TrackIsolationVariables.root", path=path)
with b2test_utils.clean_working_directory():
    with b2test_utils.set_loglevel(b2.LogLevel.INFO):
        b2.process(path)

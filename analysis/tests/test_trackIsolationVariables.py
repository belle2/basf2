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
import b2test_utils

path = b2.create_path()

b2test_utils.configure_logging_for_tests()
b2.set_random_seed("1337")

ma.inputMdstList("default",
                 filelist=[b2test_utils.require_file("mdst14.root", "validation")],
                 entrySequences=["0:1"],
                 path=path)

ntup_vars = []
for det in ("CDC", "PID", "ECL", "KLM"):
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
                                   "CDC", "PID", "ECL", "KLM",
                                   alias="dist3DToClosestTrkAtSurface")
        ma.calculateTrackIsolation(pname,
                                   path,
                                   "CDC", "PID", "ECL", "KLM",
                                   use2DRhoPhiDist=True,
                                   alias="dist2DRhoPhiToClosestTrkAtSurface")

        ma.printList(pname, full=True, path=path)
        ma.printVariableValues(pname, ntup_vars, path=path)

        ma.variablesToNtuple(pname,
                             ntup_vars,
                             treename=f"{ptype}{ch}",
                             filename="TrackIsolationVariables.root", path=path)

with b2test_utils.clean_working_directory():
    with b2test_utils.set_loglevel(b2.LogLevel.INFO):
        b2.process(path)

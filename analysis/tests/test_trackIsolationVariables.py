import basf2 as b2
import modularAnalysis as ma
import stdCharged as stdc
import b2test_utils

path = b2.create_path()

ma.inputMdstList("default",
                 filelist=[b2.find_file("mdst14.root", "validation")],
                 entrySequences=["0:10"],
                 path=path)

ntup_vars = []
for det in ("CDC", "PID", "ECL", "KLM"):
    ntup_vars.extend([f"dist3DToClosestTrkAtSurface{det}", f"dist2DRhoPhiToClosestTrkAtSurface{det}"])

for ch_name in stdc._chargednames:

    stdc.stdCharged(ch_name, "all", path)

    ma.calculateTrackIsolation(f"{ch_name}+:all",
                               path,
                               "CDC", "PID", "ECL", "KLM",
                               alias="dist3DToClosestTrkAtSurface")
    ma.calculateTrackIsolation(f"{ch_name}+:all",
                               path,
                               "CDC", "PID", "ECL", "KLM",
                               use2DRhoPhiDist=True,
                               alias="dist2DRhoPhiToClosestTrkAtSurface")

    ma.variablesToNtuple(f"{ch_name}+:all", ntup_vars, treename=ch_name, filename="TrackIsolationVariables.root", path=path)

with b2test_utils.clean_working_directory():
    with b2test_utils.set_loglevel(b2.LogLevel.DEBUG):
        b2.process(path)

#!/usr/bin/env python3

import basf2 as b2
import modularAnalysis as ma
from variables import variables as vm

main = b2.Path()

ma.inputMdst(
    b2.find_file("starterkit/2021/fei_skimmed_xulnu.udst.root", "examples"),
    path=main,
)

good_track = (
    "dr < 0.5 and abs(dz) < 2 and nCDCHits > 20 and thetaInCDCAcceptance"
)

ma.fillParticleList("mu-", "muonID > 0.9 and " + good_track, path=main)
ma.fillParticleList("pi-", "pionID > 0.5 and " + good_track, path=main)  # [E60]

ma.reconstructDecay("B0:signal -> pi- mu+ ?nu", cut="", path=main)

ma.reconstructDecay(  # [S70]
    "Upsilon(4S):opposite_cp -> B0:generic anti-B0:signal", cut="", path=main
)
ma.reconstructDecay(
    decayString="Upsilon(4S):same_cp -> B0:generic B0:signal",
    cut="",
    path=main,
)
# Combine the two Upsilon(4S) lists to one. Note: Duplicates are removed.
ma.copyLists(
    outputListName="Upsilon(4S)",
    inputListNames=["Upsilon(4S):opposite_cp", "Upsilon(4S):same_cp"],
    path=main,
)  # [E70]

ma.buildRestOfEvent("Upsilon(4S)", path=main)  # [S80]
track_based_cuts = "thetaInCDCAcceptance and pt > 0.075 and dr < 2 and abs(dz) < 4"
ecl_based_cuts = "thetaInCDCAcceptance and E > 0.05"
roe_mask = ("my_mask", track_based_cuts, ecl_based_cuts)
ma.appendROEMasks("Upsilon(4S)", [roe_mask], path=main)  # [E80]

ma.matchMCTruth(list_name="Upsilon(4S)", path=main)  # [S90]

vm.addAlias("Btag_SigProb", "daughter(0, extraInfo(SignalProbability))")
vm.addAlias("Btag_decayModeID", "daughter(0, extraInfo(decayModeID))")
vm.addAlias("Btag_Mbc", "daughter(0, Mbc)")

vm.addAlias("Bsig_isSignal",
            "daughter(1, isSignal)")
vm.addAlias("nCharged", "nROE_Charged(my_mask)")

ma.variablesToNtuple(
    "Upsilon(4S)",
    variables=[
        "Btag_SigProb",
        "Btag_decayModeID",
        "Btag_Mbc",
        "Bsig_isSignal",
        "isSignal",
        "m2RecoilSignalSide",
        "nCharged"
    ],
    filename='Upsilon4S.root',
    path=main,
)
# Process events
b2.process(main)  # [E90]
print(b2.statistics)

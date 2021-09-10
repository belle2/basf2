#!/usr/bin/env python3

import basf2 as b2
import modularAnalysis as ma
import stdPi0s

# Perform analysis.
main = b2.create_path()

ma.inputMdstList(
    filelist=[
        b2.find_file(
            "starterkit/2021/B02ks0pi0_sample.root", data_type="examples"
        ),
        b2.find_file("starterkit/2021/uubar_sample.root", data_type="examples"),
    ],
    path=main,
)

stdPi0s.stdPi0s(path=main, listtype="eff60_May2020")
ma.fillParticleList(
    decayString="pi+:good", cut="chiProb > 0.001 and pionID > 0.5", path=main
)

ma.reconstructDecay(
    decayString="K_S0 -> pi+:good pi-:good", cut="0.480<=M<=0.516", path=main
)

ma.reconstructDecay(
    decayString="B0  -> K_S0 pi0:eff60_May2020",
    cut="5.1 < Mbc < 5.3 and abs(deltaE) < 2",
    path=main,
)
#  [S10]
ma.buildRestOfEvent(target_list_name="B0", path=main)
cleanMask = (
    "cleanMask",
    "nCDCHits > 0 and useCMSFrame(p)<=3.2",
    "p >= 0.05 and useCMSFrame(p)<=3.2",
)
ma.appendROEMasks(list_name="B0", mask_tuples=[cleanMask], path=main)

ma.buildContinuumSuppression(list_name="B0", roe_mask="cleanMask", path=main)  # [E10]

simpleCSVariables = [  # [S20]
    "R2",
    "thrustBm",
    "thrustOm",
    "cosTBTO",
    "cosTBz",
]

ma.variablesToNtuple(
    decayString="B0",
    variables=simpleCSVariables + ["Mbc", "isContinuumEvent"],
    filename="ContinuumSuppression.root",
    treename="tree",
    path=main,
)

b2.process(main)  # [E20]

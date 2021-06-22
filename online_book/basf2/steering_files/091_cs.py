#!/usr/bin/env python3
import basf2 as b2
import modularAnalysis as ma
import stdPi0s

# Perform analysis.
main = b2.create_path()

ma.inputMdstList(
    environmentType="default",
    filelist=[
        b2.find_file("starterkit/2021/B02ks0pi0_sample.root",
                     data_type="examples"),
        b2.find_file("starterkit/2021/uubar_sample.root",
                     data_type="examples"),
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

ma.buildRestOfEvent(target_list_name="B0", path=main)
cleanMask = (
    "cleanMask",
    "nCDCHits > 0 and useCMSFrame(p)<=3.2",
    "p >= 0.05 and useCMSFrame(p)<=3.2",
)
ma.appendROEMasks(list_name="B0", mask_tuples=[cleanMask], path=main)

ma.buildContinuumSuppression(list_name="B0", roe_mask="cleanMask", path=main)

simpleCSVariables = [
    "R2",
    "thrustBm",
    "thrustOm",
    "cosTBTO",
    "cosTBz",
    "KSFWVariables(et)",
    "KSFWVariables(mm2)",
    "KSFWVariables(hso00)",
    "KSFWVariables(hso01)",
    "KSFWVariables(hso02)",
    "KSFWVariables(hso03)",
    "KSFWVariables(hso04)",
    "KSFWVariables(hso10)",
    "KSFWVariables(hso12)",
    "KSFWVariables(hso14)",
    "KSFWVariables(hso20)",
    "KSFWVariables(hso22)",
    "KSFWVariables(hso24)",
    "KSFWVariables(hoo0)",
    "KSFWVariables(hoo1)",
    "KSFWVariables(hoo2)",
    "KSFWVariables(hoo3)",
    "KSFWVariables(hoo4)",
    "CleoConeCS(1)",
    "CleoConeCS(2)",
    "CleoConeCS(3)",
    "CleoConeCS(4)",
    "CleoConeCS(5)",
    "CleoConeCS(6)",
    "CleoConeCS(7)",
    "CleoConeCS(8)",
    "CleoConeCS(9)"
]

ma.variablesToNtuple(
    decayString="B0",
    variables=simpleCSVariables + ["Mbc", "isContinuumEvent"],
    filename="ContinuumSuppression.root",
    treename="tree",
    path=main,
)

b2.process(main)

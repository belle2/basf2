# Doxygen should skip this script
# @cond

"""
An example script showing how to reconstruct D* mesons inclusively from slow
pions.
"""

import basf2
import modularAnalysis as mA
import stdPi0s


main_path = basf2.create_path()

mA.inputMdstList(
    environmentType="default",
    filelist=[basf2.find_file("analysis/tests/mdst.root")],
    path=main_path,
)

list_tree_tuples = list()

# MC Truth
mA.fillParticleListFromMC("pi+:from_mc", cut="[dr < 2] and [abs(dz) < 4]", addDaughters=True, path=main_path)
mA.addInclusiveDstarReconstruction(
    "D*+:Dstcharged_slowPicharged_MC -> pi+:from_mc",
    slowPionCut="[useCMSFrame(p) < 2]",
    DstarCut="useCMSFrame(p) < 2",
    path=main_path)

mA.fillParticleListFromMC("pi0:from_mc", "", addDaughters=True, path=main_path)
mA.addInclusiveDstarReconstruction(
    "D*+:Dstcharged_slowPizero_MC -> pi0:from_mc",
    slowPionCut="[useCMSFrame(p) < 2]",
    DstarCut="useCMSFrame(p) < 2",
    path=main_path)
mA.addInclusiveDstarReconstruction(
    "D*0:Dstzero_slowPizero_MC -> pi0:from_mc",
    slowPionCut="[useCMSFrame(p) < 2]",
    DstarCut="useCMSFrame(p) < 2",
    path=main_path)
mA.matchMCTruth("D*+:Dstcharged_slowPizero_MC", path=main_path)
mA.matchMCTruth("D*0:Dstzero_slowPizero_MC", path=main_path)
mA.matchMCTruth("D*+:Dstcharged_slowPicharged_MC", path=main_path)

list_tree_tuples.append(("D*+:Dstcharged_slowPicharged_MC", "Dstcharged_slowPicharged_MC"))
list_tree_tuples.append(("D*+:Dstcharged_slowPizero_MC", "Dstcharged_slowPizero_MC"))
list_tree_tuples.append(("D*0:Dstzero_slowPizero_MC", "Dstzero_slowPizero_MC"))

# Reco Level
mA.fillParticleList("pi+:slow", cut="[dr < 2] and [abs(dz) < 4]",  path=main_path)
mA.matchMCTruth("pi+:slow", path=main_path)

mA.addInclusiveDstarReconstruction(
    "D*+:Dstcharged_slowPicharged -> pi+:slow",
    slowPionCut="[useCMSFrame(p) < 2]",
    DstarCut="useCMSFrame(p) < 2",
    path=main_path)
mA.matchMCTruth("D*+:Dstcharged_slowPicharged", path=main_path)

mA.addInclusiveDstarReconstruction(
    "D*-:Dstcharged_slowPicharged_negative -> pi-:slow",
    slowPionCut="[useCMSFrame(p) < 2]",
    DstarCut="useCMSFrame(p) < 2",
    path=main_path)
mA.matchMCTruth("D*-:Dstcharged_slowPicharged_negative", path=main_path)

stdPi0s.stdPi0s("eff60_May2020", path=main_path)
mA.matchMCTruth("pi0:eff60_May2020", path=main_path)

mA.addInclusiveDstarReconstruction(
    "D*+:Dstcharged_slowPizero -> pi0:eff60_May2020",
    slowPionCut="[useCMSFrame(p) < 2]",
    DstarCut="useCMSFrame(p) < 2",
    path=main_path)
mA.matchMCTruth("D*+:Dstcharged_slowPizero", path=main_path)

mA.addInclusiveDstarReconstruction(
    "D*0:Dstzero_slowPizero -> pi0:eff60_May2020",
    slowPionCut="[useCMSFrame(p) < 2]",
    DstarCut="useCMSFrame(p) < 2",
    path=main_path)
mA.matchMCTruth("D*0:Dstzero_slowPizero", path=main_path)

list_tree_tuples.append(("D*+:Dstcharged_slowPicharged", "Dstcharged_slowPicharged"))
list_tree_tuples.append(("D*-:Dstcharged_slowPicharged_negative", "Dstcharged_slowPicharged_negative"))
list_tree_tuples.append(("D*+:Dstcharged_slowPizero", "Dstcharged_slowPizero"))
list_tree_tuples.append(("D*0:Dstzero_slowPizero", "Dstzero_slowPizero"))

dstar_variables = [
    "isSignal",
    "PDG",
    "daughter(0, PDG)",
    "E",
    "p",
    "pt",
    "useCMSFrame(p)",
    "useCMSFrame(E)",
    "matchedMC(p)",
    "matchedMC(pt)",
    "matchedMC(E)",
    "matchedMC(useCMSFrame(p))",
    "matchedMC(PDG)",
    "px",
    "py",
    "pz",
    "M",
    "InvM",
    "daughter(0, isSignal)",
    "daughter(0, E)",
    "daughter(0, p)",
    "daughter(0, useCMSFrame(p))",
    "daughter(0, matchedMC(p))",
    "daughter(0, matchedMC(E))",
    "daughter(0, matchedMC(useCMSFrame(p)))",
    "daughter(0, matchedMC(PDG))",
    "daughter(0, daughter(0, matchedMC(PDG)))",
    "daughter(0, daughter(1, matchedMC(PDG)))",
    "daughter(0, mcMother(PDG))",
    "daughter(0, mcMother(p))",
    "daughter(0, mcMother(useCMSFrame(p)))",
    "daughter(0, mcMother(E))",
    "daughter(0, mcMother(mcDaughter(0, PDG)))",
    "daughter(0, mcMother(mcDaughter(0, p)))",
    "daughter(0, mcMother(mcDaughter(0, E)))",
    "daughter(0, mcMother(mcDaughter(0, useCMSFrame(p))))",
    "daughter(0, mcMother(mcDaughter(1, PDG)))",
    "daughter(0, mcMother(mcDaughter(1, p)))",
    "daughter(0, mcMother(mcDaughter(1, E)))",
    "daughter(0, mcMother(mcDaughter(1, useCMSFrame(p))))",
    "daughter(0, mcMother(mcMother(mcDaughter(0, PDG))))",
    "daughter(0, mcMother(mcMother(mcDaughter(1, PDG))))",
]

for list_tree_tuple in list_tree_tuples:
    mA.variablesToNtuple(
        decayString=list_tree_tuple[0],
        variables=dstar_variables,
        treename=list_tree_tuple[1],
        path=main_path
    )

basf2.process(main_path)
print(basf2.statistics)

# @endcond

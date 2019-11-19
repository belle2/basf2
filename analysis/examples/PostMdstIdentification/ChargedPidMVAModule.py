#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""

cd $BELLE2_LOCAL_DIR
basf2 -i /PATH/TO/MDST/FILE.root analysis/examples/PostMdstIdentification/ChargedPidMVAModule.py -- [OPTIONS]

Input: *_mdst_*.root
Output: *_ntup_*.root

Example steering file - 2019 Belle II Collaboration.
"""

__author__ = "Marco Milesi"
__email__ = "marco.milesi@unimelb.edu.au"


import os
import sys
import argparse

g_binaryOpts = [(0, 0), (11, 211), (13, 211), (211, 321), (321, 211), (2212, 211)]


def argparser():

    description = """This steering file fills an NTuple with the ChargedPidMVA score
for charged particle identification. By default, global PID info is stored,
meaning one signal hypothesis is tested against all others.
Optionally, binary PID can be stored, by testing one (or more) pair of (S,B) mass hypotheses."""

    parser = argparse.ArgumentParser(description=description, usage=__doc__)

    def sb_pair(arg):
        try:
            s, b = map(int, arg.split(','))
            return s, b
        except BaseException:
            raise argparse.ArgumentTypeError("Option string must be of the form 'S,B'")

    parser.add_argument("--matchTruth",
                        action="store_true",
                        default=False,
                        help="Apply truth-matching on particles.")
    parser.add_argument(
        "--testHyposPDGCodePair",
        required=(
            "binary" in sys.argv),
        type=sb_pair,
        nargs='+',
        default=(
            0,
            0),
        choices=g_binaryOpts,
        help="""Use in binary mode. A list of pdgId pairs of the (S, B) charged stable particle mass hypotheses to test.
        Pass a space-separated list of (>= 1) S,B pdgIds, e.g. '--testHyposPDGCodePair 11,211 13,211'""")
    parser.add_argument("-d", "--debug",
                        dest="debug",
                        action="store",
                        default=0,
                        type=int,
                        choices=list(range(11, 20)),
                        help="Run the ChargedPidMVA module in debug mode. Pass the desired DEBUG level integer.")

    return parser


if __name__ == '__main__':

    args = argparser().parse_args()

    import basf2
    from modularAnalysis import fillParticleLists, applyChargedPidMVA, variablesToNtuple, matchMCTruth, applyCuts
    from variables import variables
    from ROOT import Belle2

    # TEMP
    # basf2.conditions.append_testing_payloads(os.path.join(os.getenv("BELLE2_LOCAL_DIR"), "localdb/database.txt"))
    basf2.conditions.append_globaltag("test_chargedpidmva_Dec2019_v0")
    print(basf2.conditions.globaltags)

    # ------------
    # Create path.
    # ------------

    path = basf2.create_path()

    # --------------------------
    # Add RootInput to the path.
    # --------------------------

    rootinput = basf2.register_module("RootInput")
    path.add_module(rootinput)

    # ---------------------------------------
    # Load standard charged stable particles,
    # and fill corresponding particle lists.
    # ---------------------------------------

    std_charged = {
        Belle2.Const.electron.getPDGCode(): {"EVTGEN_ID": "e",  "NAME": "electron", "CUT": ""},
        Belle2.Const.muon.getPDGCode(): {"EVTGEN_ID": "mu", "NAME": "muon",     "CUT": ""},
        Belle2.Const.pion.getPDGCode(): {"EVTGEN_ID": "pi", "NAME": "pion",     "CUT": ""},
        Belle2.Const.kaon.getPDGCode(): {"EVTGEN_ID": "K",  "NAME": "kaon",     "CUT": ""},
        Belle2.Const.proton.getPDGCode(): {"EVTGEN_ID": "p",  "NAME": "proton",   "CUT": ""},
    }

    plists = [(f"{d.get('EVTGEN_ID')}+:{d.get('NAME')}s", d.get("CUT")) for d in std_charged.values()]

    fillParticleLists(plists, path=path)

    # --------------------------
    # (Optional) truth matching.
    # --------------------------

    if args.matchTruth:
        for plistname, _ in plists:
            matchMCTruth(plistname, path=path)
            applyCuts(plistname, "isSignal == 1", path=path)

    # -------------------
    # Global/Binary PID ?
    # -------------------

    global_pid = (args.testHyposPDGCodePair == (0, 0))
    binary_pid = not global_pid

    # ----------------------------------------------------------------------------
    # Set variable aliases to be consistent with the names in the MVA weightfiles.
    # ----------------------------------------------------------------------------

    variables.addAlias("__event__", "evtNum")
    for det in ["SVD", "CDC", "TOP", "ARICH", "ECL", "KLM"]:
        if global_pid:
            for pdgId, d in std_charged.items():
                variables.addAlias(f"{d.get('NAME')}LogL_{det}", f"pidLogLikelihoodValueExpert({pdgId}, {det})")
        elif binary_pid:
            for s, b in args.testHyposPDGCodePair:
                s_id = std_charged.get(s).get("EVTGEN_ID")
                b_id = std_charged.get(b).get("EVTGEN_ID")
                variables.addAlias(f"deltaLogL_{s_id}_{b_id}_{det}", f"pidDeltaLogLikelihoodValueExpert({s}, {b}, {det})")

    # ----------------------
    # Apply charged Pid MVA.
    # ----------------------

    if global_pid:
        applyChargedPidMVA(particleLists=[plistname for plistname, _ in plists],
                           path=path,
                           payloadName="ChargedPidMVAWeights_Multiclass")
    elif binary_pid:
        for s, b in args.testHyposPDGCodePair:
            applyChargedPidMVA(particleLists=[plistname for plistname, _ in plists],
                               path=path,
                               binaryHypoPDGCodes=(s, b),
                               payloadName="ChargedPidMVAWeights_Classification")

    if args.debug:
        for m in path.modules():
            if "ChargedPidMVA" in m.name():
                m.logging.log_level = basf2.LogLevel.DEBUG
                m.logging.debug_level = args.debug

    # ---------------
    # Make an NTuple.
    # ---------------

    if global_pid:
        append = "_vs_".join(map(str, std_charged.keys()))
    elif binary_pid:
        append = "__".join([f"{s}_vs_{b}" for s, b in args.testHyposPDGCodePair])

    filename = f"chargedpid_ntuples__{append}.root"

    for plistname, _ in plists:
        if global_pid:
            variablesToNtuple(decayString=plistname,
                              variables=[f"pidChargedBDTScore({pdgId})" for pdgId in std_charged],
                              treename=plistname.split(':')[1],
                              filename=filename,
                              path=path)
        elif binary_pid:
            variablesToNtuple(decayString=plistname,
                              variables=[f"pidPairChargedBDTScore({s},{b})" for s, b in args.testHyposPDGCodePair],
                              treename=plistname.split(':')[1],
                              filename=filename,
                              path=path)

    # -----------------
    # Monitor progress.
    # -----------------

    progress = basf2.register_module("Progress")
    path.add_module(progress)

    # ---------------
    # Process events.
    # ---------------

    # Start processing of modules.
    basf2.process(path)

    # Print basf2 call statistics.
    print(basf2.statistics)

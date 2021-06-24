#!/usr/bin/env python3

# Doxygen should skip this script
# @cond

"""
This steering file fills an NTuple with the ChargedPidMVA score
for charged particle identification. By default, global PID info is stored,
meaning one signal hypothesis is tested against all others.
Optionally, binary PID can be stored, by testing one (or more) pair of (S,B) mass hypotheses.

Usage:

basf2 -i /PATH/TO/MDST/FILE.root analysis/examples/PostMdstIdentification/ChargedPidMVAModule.py -- [OPTIONS]

Input: *_mdst_*.root
Output: *_ntup_*.root

Example steering file - 2019 Belle II Collaboration.
"""

__author__ = "Marco Milesi"
__email__ = "marco.milesi@unimelb.edu.au"


import argparse
import re
from modularAnalysis import getAnalysisGlobaltag


def argparser():

    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawTextHelpFormatter)

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
    parser.add_argument("--testHyposPDGCodePair",
                        type=sb_pair,
                        nargs='+',
                        default=(0, 0),
                        help="Option required in binary mode.\n"
                        "A list of pdgId pairs of the (S, B) charged stable particle mass hypotheses to test.\n"
                        "Pass a space-separated list of (>= 1) S,B pdgIds, e.g.:\n"
                        "'--testHyposPDGCodePair 11,211 13,211'")
    parser.add_argument("--addECLOnly",
                        dest="add_ecl_only",
                        action="store_true",
                        default=False,
                        help="Apply the BDT also for the ECL-only training."
                        "This will result in a separate score branch in the ntuple.")
    parser.add_argument("--chargeIndependent",
                        action="store_true",
                        default=False,
                        help="Use a BDT trained on a sample of inclusively charged particles.")
    parser.add_argument("--global_tag_append",
                        type=str,
                        nargs="+",
                        default=[getAnalysisGlobaltag()],
                        help="List of names of conditions DB global tag(s) to append on top of GT replay.\n"
                        "NB: these GTs will have lowest priority.\n"
                        "Pass a space-separated list of names.")
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
    import modularAnalysis as ma
    from ROOT import Belle2
    import pdg

    for tag in args.global_tag_append:
        basf2.conditions.append_globaltag(tag)
    print(f"Appending GTs:\n{args.global_tag_append}")

    # ------------
    # Create path.
    # ------------

    path = basf2.create_path()

    # ----------
    # Add input.
    # ----------

    ma.inputMdst(environmentType="default",
                 filename=basf2.find_file("mdst13.root", "validation"),
                 path=path)

    # ---------------------------------------
    # Load standard charged stable particles,
    # and fill corresponding particle lists.
    # ---------------------------------------

    std_charged = [
        Belle2.Const.electron.getPDGCode(),
        Belle2.Const.muon.getPDGCode(),
        Belle2.Const.pion.getPDGCode(),
        Belle2.Const.kaon.getPDGCode(),
        Belle2.Const.proton.getPDGCode(),
        Belle2.Const.deuteron.getPDGCode(),
    ]

    plists = [(f"{pdg.to_name(pdgId)}:my_{pdg.to_name(pdgId)}", "") for pdgId in std_charged]
    ma.fillParticleLists(plists, path=path)

    # --------------------------
    # (Optional) truth matching.
    # --------------------------

    if args.matchTruth:
        for plistname, _ in plists:
            ma.matchMCTruth(plistname, path=path)
            applyCuts(plistname, "isSignal == 1", path=path)

    # -------------------
    # Global/Binary PID ?
    # -------------------

    global_pid = (args.testHyposPDGCodePair == (0, 0))
    binary_pid = not global_pid

    # ----------------------
    # Apply charged Pid MVA.
    # ----------------------

    if global_pid:
        ma.applyChargedPidMVA(particleLists=[plistname for plistname, _ in plists],
                              path=path,
                              trainingMode=Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode.c_Multiclass,
                              chargeIndependent=args.chargeIndependent)
        if args.add_ecl_only:
            ma.applyChargedPidMVA(particleLists=[plistname for plistname, _ in plists],
                                  path=path,
                                  trainingMode=Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode.c_ECL_Multiclass)
    elif binary_pid:
        for s, b in args.testHyposPDGCodePair:
            ma.applyChargedPidMVA(particleLists=[plistname for plistname, _ in plists],
                                  path=path,
                                  trainingMode=Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode.c_Classification,
                                  binaryHypoPDGCodes=(s, b),
                                  chargeIndependent=args.chargeIndependent)
            if args.add_ecl_only:
                ma.applyChargedPidMVA(particleLists=[plistname for plistname, _ in plists],
                                      path=path,
                                      trainingMode=Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode.c_ECL_Classification,
                                      binaryHypoPDGCodes=(s, b))

    if args.debug:
        for m in path.modules():
            if "ChargedPidMVA" in m.name():
                m.logging.log_level = basf2.LogLevel.DEBUG
                m.logging.debug_level = args.debug

    # ---------------
    # Make an NTuple.
    # ---------------

    if global_pid:

        append = "_vs_".join(map(str, std_charged))

        variables = [f"pidChargedBDTScore({pdgId}, ALL)" for pdgId in std_charged]
        if args.add_ecl_only:
            variables += [f"pidChargedBDTScore({pdgId}, ECL)" for pdgId in std_charged]

    elif binary_pid:

        append = "__".join([f"{s}_vs_{b}" for s, b in args.testHyposPDGCodePair])

        variables = [f"pidPairChargedBDTScore({s}, {b}, ALL)" for s, b in args.testHyposPDGCodePair]
        if args.add_ecl_only:
            variables += [f"pidPairChargedBDTScore({s}, {b}, ECL)" for s, b in args.testHyposPDGCodePair]

    filename = f"chargedpid_ntuples__{append}.root"

    for plistname, _ in plists:

        # ROOT doesn't like non-alphanum chars...
        treename = re.sub(r"[\W]+", "", plistname.split(':')[1])

        if global_pid:
            ma.variablesToNtuple(decayString=plistname,
                                 variables=variables,
                                 treename=treename,
                                 filename=filename,
                                 path=path)
        elif binary_pid:
            ma.variablesToNtuple(decayString=plistname,
                                 variables=variables,
                                 treename=treename,
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

# @endcond

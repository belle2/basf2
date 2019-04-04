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

import argparse


def argparser():

    description = """This steering file fills an NTuple with the (currently ECL-only-based) ChargedPidMVA score
for a given pair of (S,B) mass hypotheses for charged stable particles. Multiple (S,B) pairs can be tested at the same time."""

    parser = argparse.ArgumentParser(description=description, usage=__doc__)

    def sb_pair(arg):
        try:
            s, b = map(int, arg.split(','))
            return s, b
        except BaseException:
            raise argparse.ArgumentTypeError("Option string must be of the form \'S,B\'")

    parser.add_argument("--testHyposPDGCodePair",
                        dest="testHyposPDGCodePair",
                        required=True,
                        type=sb_pair,
                        nargs='+',
                        help="""A list of pdgId pairs of the (signal,background) charged stable
particle mass hypotheses to test. Pass a space-separated list of (>= 1) S,B pdgIds,
e.g. \'--testHyposPDGCodePair 11,211 13,211\'""")
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
    from modularAnalysis import fillParticleLists, applyChargedPidMVA, variablesToNtuple

    # ------------
    # Create path.
    # ------------

    path = basf2.create_path()

    # --------------------------
    # Add RootInput to the path.
    # --------------------------

    rootinput = basf2.register_module('RootInput')
    path.add_module(rootinput)

    # ---------------------------------------
    # Load standard charged stable particles,
    # and fill corresponding particle lists.
    # ---------------------------------------

    electrons = ('e+:electrons', '')
    muons = ('mu+:muons', '')
    pions = ('pi+:pions', '')
    kaons = ('K+:kaons', '')
    protons = ('p+:protons', '')

    plists = [electrons, muons, pions, kaons, protons]

    fillParticleLists(plists, path=path)

    # Set variable aliases if needed be.
    from variables import variables
    variables.addAlias("clusterEop", "clusterEoP")
    variables.addAlias("eclPulseShapeDiscriminationMVA", "clusterPulseShapeDiscriminationMVA")
    variables.addAlias("__event__", "evtNum")

    # ----------------------
    # Apply charged Pid MVA.
    # ----------------------

    for s, b in args.testHyposPDGCodePair:
        applyChargedPidMVA(sigHypoPDGCode=s,
                           bkgHypoPDGCode=b,
                           particleLists=[plist[0] for plist in plists],
                           path=path)

    if args.debug:
        for m in path.modules():
            if "ChargedPidMVA" in m.name():
                m.logging.log_level = basf2.LogLevel.DEBUG
                m.logging.debug_level = args.debug

    # ---------------
    # Make an NTuple.
    # ---------------

    append = "__".join([f"{s}_vs_{b}" for s, b in args.testHyposPDGCodePair])
    filename = f"chargedpid_ntuples__{append}.root"
    for plist in plists:
        variablesToNtuple(decayString=plist[0],
                          variables=[f"pidPairChargedBDTScore({s},{b})" for s, b in args.testHyposPDGCodePair],
                          treename=plist[0].split(':')[1],
                          filename=filename,
                          path=path)

    # -----------------
    # Monitor progress.
    # -----------------

    progress = basf2.register_module('Progress')
    path.add_module(progress)

    # ---------------
    # Process events.
    # ---------------

    # Start processing of modules.
    basf2.process(path)

    # Print basf2 call statistics.
    print(basf2.statistics)

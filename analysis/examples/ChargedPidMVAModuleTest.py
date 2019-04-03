#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""

cd $BELLE2_LOCAL_DIR
basf2 -i /PATH/TO/MDST/FILE.root analysis/examples/ChargedPidMVAModuleTest.py -- [OPTIONS]

Input: *_mdst_*.root
Output: *_ntup_*.root

Example steering file - 2019 Belle II Collaboration.
"""

__author__ = "Marco Milesi"
__email__ = "marco.milesi@unimelb.edu.au"

import os
import argparse


def argparser():

    description = """This steering file fills an NTuple with the ChargedPidMVA score
for a given pair of (S,B) mass hypotheses for charged stable particles."""

    parser = argparse.ArgumentParser(description=description, usage=__doc__)

    parser.add_argument("--sigPdgId",
                        dest="sigPdgId",
                        required=True,
                        type=int,
                        help="The pdgId of the signal charged stable particle mass hypothesis.")
    parser.add_argument("--bkgPdgId",
                        dest="bkgPdgId",
                        required=True,
                        type=int,
                        help="The pdgId of the background charged stable particle mass hypothesis.")
    parser.add_argument("-d", "--debug",
                        dest="debug",
                        action="store",
                        default=0,
                        type=int,
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

    applyChargedPidMVA(sigPdgId=args.sigPdgId,
                       bkgPdgId=args.bkgPdgId,
                       particleLists=[plist[0] for plist in plists],
                       path=path)

    if args.debug:
        # Set debug level for this module
        for m in path.modules():
            if m.name() == "ChargedPidMVA":
                m.logging.log_level = basf2.LogLevel.DEBUG
                m.logging.debug_level = args.debug

    # ---------------
    # Make an NTuple.
    # ---------------

    filename = "chargedpid_ntuples.root"
    for plist in plists:
        variablesToNtuple(decayString=plist[0],
                          variables=[f"chargedPidBDT({args.sigPdgId},{args.bkgPdgId})"],
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

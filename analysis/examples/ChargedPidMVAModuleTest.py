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

    description = "This steering file fills an NTuple with the ChargedPidMVA score for a given pair of (S,B) mass hypotheses."

    parser = argparse.ArgumentParser(description=description, usage=__doc__)

    sigPdgIds = [11, 13, 211, 321, 2212]
    bkgPdgIds = [211, 321]

    parser.add_argument("--sigPdgId",
                        dest="sigPdgId",
                        required=True,
                        type=int,
                        choices=sigPdgIds,
                        help="The pdgId of the signal mass hypothesis.")
    parser.add_argument("--bkgPdgId",
                        dest="bkgPdgId",
                        required=True,
                        type=int,
                        choices=bkgPdgIds,
                        help="The pdgId of the background mass hypothesis.")

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

    # --------------------------------------
    # Load standard charged particles,
    # and fill corresponding particle lists.
    # --------------------------------------

    electrons = ('e+:electrons', '')
    muons = ('mu+:muons', '')
    pions = ('pi+:pions', '')
    kaons = ('K+:kaons', '')
    protons = ('p+:protons', '')

    plists = [electrons, muons, pions, kaons, protons]

    fillParticleLists(plists, path=path)

    # TEMP: set aliases
    from variables import variables
    variables.addAlias("clusterEop", "clusterEoP")
    variables.addAlias("eclPulseShapeDiscriminationMVA", "clusterPulseShapeDiscriminationMVA")
    variables.addAlias("__event__", "evtNum")

    # ----------------------
    # Apply charged Pid MVA.
    # ----------------------

    applyChargedPidMVA(sigPdgId=args.sigPdgId,
                       bkgPdgId=args.bkgPdgId,
                       path=path)

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

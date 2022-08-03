#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Doxygen should skip this script
# @cond

"""
Example script to calculate track isolation variables.

For each particle's track in the input charged stable particle list,
calculate the minimal distance to the other candidates' tracks at a given detector surface.
"""

import argparse


def argparser():
    """ Argument parser
    """

    import stdCharged as stdc

    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument("--std_charged_ref", type=str, choices=stdc._chargednames, default="pi",
                        help="The base name of the reference standard charged particle list\n"
                        "against which to calculate the distance.")
    parser.add_argument("--detectors",
                        type=str,
                        nargs="+",
                        default=["CDC", "TOP", "ARICH", "ECL", "KLM"],
                        choices=["CDC", "TOP", "ARICH", "ECL", "KLM"],
                        help="List of detectors at whose entry surface track isolation variables will be calculated.\n"
                        "Pass a space-separated list of names.")
    parser.add_argument("-d", "--debug",
                        action="store",
                        default=0,
                        type=int,
                        choices=list(range(11, 20)),
                        help="Run the TrackIsoCalculator module in debug mode. Pass the desired DEBUG level integer.")

    return parser


if __name__ == "__main__":

    # Argparse options.
    # NB: Command line arguments are parsed before importing basf2, to avoid PyROOT hijacking them
    # in case of overlapping option names.
    args = argparser().parse_args()

    import basf2 as b2
    import modularAnalysis as ma
    from variables import variables
    import variables.utils as vu
    import variables.collections as vc

    # Create path. Register necessary modules to this path.
    path = b2.create_path()

    # Add input data and ParticleLoader modules to the path.
    ma.inputMdst(filename=b2.find_file("mdst14.root", "validation"), path=path)

    # Fill a particle list of muons, with some quality selection.
    base_trk_selection = "[dr < 3] and [abs(dz) < 5] and [thetaInCDCAcceptance] and [pt > 0.1]"
    ma.fillParticleList("mu+:muons", f"{base_trk_selection} and [muonID > 0.5]", path=path)

    # Fill a reference charged stable particle list to calculate the distances against.
    # Generally, this list should have a very loose selection applied (if none at all).
    ref = f"{args.std_charged_ref}+:presel"
    ma.fillParticleList(ref, f"{base_trk_selection}", path=path)

    # Mode 1: calculate the track isolation variables
    # directly on the muons particle list.
    trackiso_vars = ma.calculateTrackIsolation("mu+:muons",
                                               path,
                                               *args.detectors,
                                               reference_list_name=ref,
                                               highest_prob_mass_for_ext=False)

    # Reconstruct the J/psi decay.
    jpsimumu = "J/psi:mumu -> mu+:muons mu-:muons"
    jpsi_cuts = [
        "[2.8 < M < 3.3]",
        "[daughterSumOf(charge) == 0]",
    ]
    jpsi_cut = " and ".join(jpsi_cuts)

    ma.reconstructDecay(jpsimumu, jpsi_cut, path=path)

    # Mode 2: calculate the track isolation variables
    # on the selected muon daughters in the decay.
    # This time, use the mass hypotheiss w/ highest probability for the track extrapolation.
    trackiso_vars_highestprobmass = ma.calculateTrackIsolation("J/psi:mumu -> ^mu+ ^mu-",
                                                               path,
                                                               *args.detectors,
                                                               reference_list_name=ref,
                                                               highest_prob_mass_for_ext=True)

    variables_jpsi = vc.kinematics + ["daughterDiffOfPhi(0, 1)"]
    variables_jpsi += vu.create_aliases(variables_jpsi, "useCMSFrame({variable})", "CMS")
    variables_jpsi += vc.inv_mass
    variables_mu = vc.kinematics + ["theta", "phi", "clusterE"] + trackiso_vars + trackiso_vars_highestprobmass + [
        "inARICHAcceptance",
        "inCDCAcceptance",
        "inECLAcceptance",
        "inKLMAcceptance",
        "inTOPAcceptance",
        "ptInBECLAcceptance",
        "ptInBKLMAcceptance",
        "ptInTOPAcceptance",
        "thetaInARICHAcceptance",
        "thetaInBECLAcceptance",
        "thetaInBKLMAcceptance",
        "thetaInCDCAcceptance",
        "thetaInECLAcceptance",
        "thetaInEECLAcceptance",
        "thetaInEKLMAcceptance",
        "thetaInKLMAcceptance",
        "thetaInKLMOverlapAcceptance",
        "thetaInTOPAcceptance"
    ]

    aliases_jpsimumu = vu.create_aliases_for_selected(
        variables_jpsi,
        "^J/psi:mumu -> mu+:muons mu-:muons",
        prefix=["jpsi"])

    aliases_mu = vu.create_aliases_for_selected(
        variables_mu,
        "J/psi:mumu -> ^mu+:muons ^mu-:muons",
        use_names=True)

    variables.addAlias("nReferenceTracks", f"nCleanedTracks({base_trk_selection})")
    aliases_event = ["nReferenceTracks"]

    variables.printAliases()

    # Saving variables to ntuple
    ma.variablesToNtuple(decayString="J/psi:mumu",
                         variables=aliases_event+aliases_jpsimumu+aliases_mu,
                         treename="jpsimumu",
                         filename="TrackIsolationVariables.root",
                         path=path)

    # Optionally activate debug mode for the TrackIsoCalculator module(s).
    if args.debug:
        for m in path.modules():
            if "TrackIsoCalculator" in m.name():
                m.set_log_level(b2.LogLevel.DEBUG)
                m.set_debug_level(args.debug)

    path.add_module("Progress")

    # Process the data.
    b2.process(path)

    print(b2.statistics)

# @endcond

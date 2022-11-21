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
Example script to calculate isolation variables per particle.

For each particle in the input charged stable particle list,
calculate the distance to the closest candidate in the reference list at a given detector layer surface.
The calculation of the distance is based on the particles' track helices extrapolation.
"""


import argparse


def argparser():
    """ Argument parser
    """

    import stdCharged as stdc
    from modularAnalysis import getAnalysisGlobaltag

    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument("--std_charged_ref",
                        type=str,
                        choices=stdc._chargednames,
                        default="pi",
                        help="The base name of the reference standard charged particle list\n"
                        "that will be considered for the distance calculation.\n"
                        "Default: %(default)s.")
    parser.add_argument("--detectors",
                        type=str,
                        nargs="+",
                        default=["CDC", "TOP", "ARICH", "ECL", "KLM"],
                        choices=["CDC", "TOP", "ARICH", "ECL", "KLM"],
                        help="List of detectors at whose entry surface the isolation variables will be calculated.\n"
                        "Pass a space-separated list of names.\n"
                        "Default: %(default)s.")
    parser.add_argument("--use_pid_det_weights",
                        action="store_true",
                        default=False,
                        help="Include the PID detector weights (taken from the CDB) in the isolation score calculation.\n"
                        "Default: %(default)s.")
    parser.add_argument("--global_tag_append",
                        type=str,
                        nargs="+",
                        default=[getAnalysisGlobaltag()],
                        help="List of names of conditions DB global tag(s) to append on top of GT replay.\n"
                        "NB: these GTs will have lowest priority over GT replay.\n"
                        "The order of the sequence passed determines the priority of the GTs, w/ the highest coming first.\n"
                        "Pass a space-separated list of names.\n"
                        "Default: %(default)s.")
    parser.add_argument("--global_tag_prepend",
                        type=str,
                        nargs="+",
                        default=None,
                        help="List of names of conditions DB global tag(s) to prepend to GT replay.\n"
                        "NB: these GTs will have highest priority over GT replay.\n"
                        "The order of the sequence passed determines the priority of the GTs, w/ the highest coming first.\n"
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
    import stdV0s as stdv0
    from variables import variables as vm
    import variables.utils as vu
    import variables.collections as vc
    import pdg
    from ROOT import Belle2
    Const = Belle2.Const

    for tag in args.global_tag_append:
        b2.conditions.append_globaltag(tag)
    print(f"Appending GTs:\n{args.global_tag_append}")

    if args.global_tag_prepend:
        for tag in reversed(args.global_tag_prepend):
            b2.conditions.prepend_globaltag(tag)
        print(f"Prepending GTs:\n{args.global_tag_prepend}")

    # Create path. Register necessary modules to this path.
    path = b2.create_path()

    # Add input data and ParticleLoader modules to the path.
    ma.inputMdst(filename=b2.find_file("mdst14.root", "validation"), path=path)

    # -------------------------------------------------------------------

    # ---------
    # EXAMPLE 1
    # ---------

    # Fill a particle list of muons, with some quality selection.
    base_trk_selection = "[dr < 3] and [abs(dz) < 5] and [thetaInCDCAcceptance] and [pt > 0.1]"
    ma.fillParticleList("mu+:muons", f"{base_trk_selection} and [muonID > 0.5]", path=path)

    # Reconstruct a J/psi decay.
    jpsimumu = "J/psi:mumu -> mu+:muons mu-:muons"
    jpsi_cuts = [
        "[2.8 < M < 3.3]",
        "[daughterSumOf(charge) == 0]",
    ]
    jpsi_cut = " and ".join(jpsi_cuts)

    ma.reconstructDecay(jpsimumu, jpsi_cut, path=path)

    # Fill a reference charged stable particle list to calculate the distances against.
    # Generally, this list should have a very loose selection applied (if none at all).
    ref = f"{args.std_charged_ref}+:ref"
    ref_pdg = pdg.from_name(f"{args.std_charged_ref}+")
    ma.fillParticleList(ref, f"{base_trk_selection}", path=path)

    # Calculate the track isolation variables
    # on the muon daughters in the decay.
    # Use the default setting where the module takes the mass hypothesis w/ highest probability for the track extrapolation.
    # The helper returns a dictionary w/ the list of metavariables for each of the reference particle lists (using PDG as key).
    # In this case, we pass explicitly a particle list of our choice.
    trackiso_vars_mu = ma.calculateTrackIsolation("J/psi:mumu -> ^mu+ ^mu-",
                                                  path,
                                                  *args.detectors,
                                                  vars_for_nearest_part=vc.mc_variables,
                                                  # Calculate also the chosen variables for the nearest particle at each layer.
                                                  reference_list_name=ref,
                                                  # Include/exclude the PID detector weights in the score calculation.
                                                  exclude_pid_det_weights=not args.use_pid_det_weights)

    # Variables and aliases for the J/psi candidates.
    variables_jpsi = vc.kinematics + ["daughterDiffOfPhi(0, 1)"]
    variables_jpsi += vu.create_aliases(variables_jpsi, "useCMSFrame({variable})", "CMS")
    variables_jpsi += vc.inv_mass
    aliases_jpsi = vu.create_aliases_for_selected(variables_jpsi,
                                                  "^J/psi:mumu -> mu+ mu-",
                                                  prefix=["jpsi"])

    # Variables and aliases for the J/psi daughters.
    # Since we passed explicitly a reference list, we use its associated PDG code to get the list of metavariables.
    variables_mu = vc.kinematics + ["theta", "phi", "clusterE", "nCDCHits"] + trackiso_vars_mu[ref_pdg]
    aliases_mu = vu.create_aliases_for_selected(variables_mu,
                                                "J/psi:mumu -> ^mu+ ^mu-",
                                                use_names=True)

    # Variables and aliases for the event.
    vm.addAlias("nReferenceTracks", f"nCleanedTracks({base_trk_selection})")
    aliases_event = ["nReferenceTracks"]

    # Saving variables to ntuple
    ma.variablesToNtuple(decayString="J/psi:mumu",
                         variables=aliases_event+aliases_jpsi+aliases_mu,
                         treename="jpsimumu",
                         filename="TrackIsolationVariables.root",
                         path=path)

    # ---------
    # EXAMPLE 2
    # ---------

    # Reconstruct standard Lambda0 -> p+ pi- decays.
    stdv0.stdLambdas(path=path)

    # Calculate the track isolation variables
    # on the proton and pion daughters in the decay.
    #
    # In this configuration, the mass hypothesis for the extrapolation is the one matching each particle's PDG.
    #
    # Note that no reference list is passed: it will use by default the `:all` ParticleList of the same type
    # of the selected particle(s) in the decay string.
    trackiso_vars_p_pi = ma.calculateTrackIsolation("Lambda0:merged -> ^p+ ^pi-",
                                                    path,
                                                    *args.detectors,
                                                    vars_for_nearest_part=vc.mc_variables,
                                                    # Calculate also the chosen variables for the nearest particle at each layer.
                                                    highest_prob_mass_for_ext=False,
                                                    exclude_pid_det_weights=not args.use_pid_det_weights)

    # Variables and aliases for the Lambda0 candidates.
    variables_lambda0 = vc.kinematics + ["daughterDiffOfPhi(0, 1)"]
    variables_lambda0 += vu.create_aliases(variables_lambda0, "useCMSFrame({variable})", "CMS")
    variables_lambda0 += vc.inv_mass
    aliases_lambda0 = vu.create_aliases_for_selected(variables_lambda0,
                                                     "^Lambda0:merged -> p+ pi-",
                                                     prefix=["lambda0"])

    # Variables and aliases for the Lambda0 daughters.
    # - Protons
    variables_p = vc.kinematics + \
        ["theta", "phi", "clusterE", "nCDCHits"] + \
        trackiso_vars_p_pi[Const.proton.getPDGCode()]  # Use the proton PDG to get the associated list of metavariables.
    aliases_p = vu.create_aliases_for_selected(variables_p,
                                               "Lambda0:merged -> ^p+ pi-",
                                               use_names=True)
    # - Pions
    variables_pi = vc.kinematics + \
        ["theta", "phi", "clusterE", "nCDCHits"] + \
        trackiso_vars_p_pi[Const.pion.getPDGCode()]  # Use the pion PDG to get the associated list of metavariables.
    aliases_pi = vu.create_aliases_for_selected(variables_pi,
                                                "Lambda0:merged -> p+ ^pi-",
                                                use_names=True)

    # Saving variables to ntuple
    ma.variablesToNtuple(decayString="Lambda0:merged",
                         variables=aliases_lambda0+aliases_p+aliases_pi,
                         treename="lambda0ppi",
                         filename="TrackIsolationVariables.root",
                         path=path)

    # -------------------------------------------------------------------

    vm.printAliases()

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

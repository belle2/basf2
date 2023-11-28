#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Example to create lepton particle list and apply LeptonID corrections to a MC sample
by retrieving lookup tables from the conditions DB.
"""

# Doxygen should skip this script
# @cond

import argparse


def argparser():

    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument("--release",
                        type=int,
                        default=5,
                        help="The major release number associated to the corrections that are being applied.\n"
                        "Default: %(default)s.")
    parser.add_argument("--global_tag_append",
                        type=str,
                        nargs="+",
                        default=['analysis_tools_light-2302-genetta'],
                        help="List of names of conditions DB global tag(s) to append on top of GT replay.\n"
                        "NB: these GTs will have lowest priority over GT replay.\n"
                        "The order of the sequence passed determines the priority of the GTs, w/ the highest coming first.\n"
                        "Pass a space-separated list of names.\n"
                        "Default: %(default)s.")
    parser.add_argument("--lid_weights_gt",
                        type=str,
                        default="leptonid_Moriond2022_Official_rel5_v1a",
                        help="Name of conditions DB global tag with recommended lepton ID correction factors.\n"
                        "Default: %(default)s.")

    return parser


def main():
    """
    Main entry point allowing external calls.
    """

    args = argparser().parse_args()

    import basf2 as b2
    import modularAnalysis as ma
    from variables import variables as vm
    import variables.utils as vu
    import variables.collections as vc
    from stdCharged import stdE, stdMu

    b2.set_log_level(b2.LogLevel.INFO)

    for tag in args.global_tag_append:
        b2.conditions.append_globaltag(tag)
    print(f"Appending GTs:\n{args.global_tag_append}")

    path = b2.create_path()

    # ----------
    # Add input.
    # ----------

    ma.inputMdst(environmentType="default",
                 filename=b2.find_file("mdst14.root", "validation"),
                 entrySequence="0:10000",
                 path=path)

    # ----------------------------------------------
    # Define preselected particle lists for leptons.
    # ----------------------------------------------

    # For electrons, we show the case in which a Bremsstrahlung correction
    # is applied first to get the 4-momentum right,
    # and the resulting particle list is passed as input to the stdE list creator.
    ma.fillParticleList("e+:uncorrected",
                        cut="dr < 2 and abs(dz) < 4",  # NB: whichever cut is set here, will be inherited by the std electrons.
                        path=path)
    ma.fillParticleList("gamma:bremsinput",
                        cut="E < 1.0",
                        path=path)
    ma.correctBremsBelle(outputListName="e+:corrected",
                         inputListName="e+:uncorrected",
                         gammaListName="gamma:bremsinput",
                         path=path)

    ma.fillParticleList("mu+:presel",
                        cut="dr < 2 and abs(dz) < 4",  # NB: whichever cut is set here, will be inherited by the std muons.
                        path=path)

    # -------------------------------------------------------------
    # Calculate track isolation variables on the preselected lists.
    # NB: this must be done *before* the sdtLep lists are defined!
    # -------------------------------------------------------------

    # Reference list for isolation variables' calculation.
    ma.fillParticleList("pi+:ref", "inCDCAcceptance", path=path)

    # Define alias for isolation score. Needed to get the correct bin in the payload.
    vm.addAlias("minET2ETIsoScore", "minET2ETIsoScore(pi+:ref, 1, CDC, TOP, ARICH, ECL, KLM)")

    _ = ma.calculateTrackIsolation("e+:corrected",
                                   path,
                                   *["CDC", "TOP", "ARICH", "ECL", "KLM"],
                                   reference_list_name="pi+:ref")

    _ = ma.calculateTrackIsolation("mu+:presel",
                                   path,
                                   *["CDC", "TOP", "ARICH", "ECL", "KLM"],
                                   reference_list_name="pi+:ref")

    # ----------------------------------
    # Fill example standard lepton list.
    # ----------------------------------

    electrons_fixed09 = "lh_B_fixed09"
    electrons_wp = "FixedThresh09"
    electron_id_var, electron_id_weights = stdE(electrons_wp, "likelihood", "binary", args.lid_weights_gt,
                                                release=args.release,
                                                inputListName="e+:corrected",
                                                outputListLabel=electrons_fixed09,
                                                path=path)

    muons_uniform90 = "bdt_G_uniform90"
    muons_wp = "UniformEff90"
    muon_id_var, muon_id_weights = stdMu(muons_wp, "bdt", "global", args.lid_weights_gt,
                                         release=args.release,
                                         inputListName="mu+:presel",
                                         outputListLabel=muons_uniform90,
                                         path=path)

    # --------------------------------------------
    # Add extra cuts on the standard lepton lists.
    # --------------------------------------------

    ma.applyCuts(f"e-:{electrons_fixed09}", "[pt > 0.1] and thetaInCDCAcceptance", path=path)
    ma.applyCuts(f"mu-:{muons_uniform90}", "[pt > 0.1] and thetaInCDCAcceptance", path=path)

    # --------------------------------------------------
    # Reconstruct J/psi candidates from the std leptons.
    # --------------------------------------------------

    jpsiee = f"J/psi:ee -> e+:{electrons_fixed09} e-:{electrons_fixed09}"
    jpsimumu = f"J/psi:mumu -> mu+:{muons_uniform90} mu-:{muons_uniform90}"

    jpsi_cuts = [
        "[2.8 < M < 3.3]",
        "[daughterSumOf(charge) == 0]",
    ]
    jpsi_cut = " and ".join(jpsi_cuts)

    ma.reconstructDecay(jpsiee, jpsi_cut, path=path)
    ma.reconstructDecay(jpsimumu, jpsi_cut, path=path)

    # ------------------------------------------
    # Create some example variable aliases
    # for the mother particle and the daughters.
    # ------------------------------------------

    variables_jpsi = []
    variables_e = []
    variables_mu = []

    variables_jpsi += vc.kinematics
    variables_jpsi += vc.inv_mass

    variables_e += (vc.kinematics + ["theta", "charge", "minET2ETIsoScore"])
    variables_mu += (vc.kinematics + ["theta", "charge", "minET2ETIsoScore"])

    cms_kinematics = vu.create_aliases(vc.kinematics, "useCMSFrame({variable})", "CMS")

    variables_e += cms_kinematics
    variables_mu += cms_kinematics

    lid_e = [electron_id_var] + electron_id_weights
    variables_e += lid_e

    lid_mu = [muon_id_var] + muon_id_weights
    variables_mu += lid_mu

    aliases_jpsiee = vu.create_aliases_for_selected(
        variables_jpsi,
        f"^J/psi:ee -> e+:{electrons_fixed09} e-:{electrons_fixed09}",
        prefix=["jpsi"])
    aliases_jpsimumu = vu.create_aliases_for_selected(
        variables_jpsi,
        f"^J/psi:mumu -> mu+:{muons_uniform90} mu-:{muons_uniform90}",
        prefix=["jpsi"])

    aliases_e = vu.create_aliases_for_selected(
        variables_e,
        f"J/psi:ee -> ^e+:{electrons_fixed09} ^e-:{electrons_fixed09}",
        use_names=True)
    aliases_mu = vu.create_aliases_for_selected(
        variables_mu,
        f"J/psi:mumu -> ^mu+:{muons_uniform90} ^mu-:{muons_uniform90}",
        use_names=True)

    vm.printAliases()

    output_file = "jpsill_LID_weights.root"

    # Saving variables to ntuple
    ma.variablesToNtuple(decayString="J/psi:ee",
                         variables=aliases_jpsiee+aliases_e,
                         treename="jpsiee",
                         filename=output_file,
                         path=path)
    ma.variablesToNtuple(decayString="J/psi:mumu",
                         variables=aliases_jpsimumu+aliases_mu,
                         treename="jpsimumu",
                         filename=output_file,
                         path=path)

    # Process the events.
    b2.process(path)

    # Print out the summary.
    print(b2.statistics)


if __name__ == "__main__":

    main()

# @endcond

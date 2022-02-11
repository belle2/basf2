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
Example to create lepton particle list and apply LeptonID corrections to a MC sample
by retrieving lookup tables from the conditions DB.
"""


import argparse


def argparser():

    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawTextHelpFormatter)

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
    from variables import variables
    import variables.utils as vu
    import variables.collections as vc
    from stdCharged import stdE, stdMu

    b2.set_log_level(b2.LogLevel.INFO)

    # Append the analysis GT. Needed to run the ChargedPidMVA BDT.
    analysis_gt = ma.getAnalysisGlobaltag()
    b2.B2INFO(f"Appending analysis GT: {analysis_gt}")
    b2.conditions.append_globaltag(analysis_gt)

    path = b2.create_path()

    # ----------
    # Add input.
    # ----------

    ma.inputMdst(environmentType="default",
                 filename=b2.find_file("mdst13.root", "validation"),
                 entrySequence="0:10000",
                 path=path)

    # ----------------------------------
    # Fill example standard lepton list.
    # ----------------------------------

    # For electrons, we show the case in which a Bremsstrahlung correction
    # is applied first to get the 4-momentum right,
    # and the resulting particle list is passed as input to the stdE list creator.
    ma.fillParticleList("e+:uncorrected",
                        cut="dr < 2 and abs(dz) < 4",  # NB: whichever cut is set here, will be inherited by the std electrons.
                        path=path)
    ma.fillParticleList("gamma:bremsinput",
                        cut="E < 1.0",
                        path=path)
    ma.correctBrems(outputList="e+:corrected",
                    inputList="e+:uncorrected",
                    gammaList="gamma:bremsinput",
                    path=path)

    electrons_fixed09 = "lh_B_fixed09"
    electron_id_var = stdE("FixedThresh09", "likelihood", "binary", args.lid_weights_gt,
                           release=5,
                           listname=electrons_fixed09,
                           input_listname="e+:corrected",
                           path=path)

    muons_uniform90 = "bdt_G_uniform90"
    muon_id_var = stdMu("UniformEff90", "bdt", "global", args.lid_weights_gt,
                        release=5,
                        listname=muons_uniform90,
                        path=path)

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

    variables_e += vc.kinematics
    variables_mu += vc.kinematics

    cms_kinematics = vu.create_aliases(vc.kinematics, "useCMSFrame({variable})", "CMS")

    variables_e += cms_kinematics
    variables_mu += cms_kinematics

    lid_e = [
        electron_id_var,
        # The following aliases for LID weights are already set when creating the standard lepton list (see ma.stdLep).
        # You can decide to alias them to something else.
        f"weight_{electron_id_var}_eff_FixedThresh09",
        f"weight_{electron_id_var}_eff_FixedThresh09_rel_stat_up",
        f"weight_{electron_id_var}_eff_FixedThresh09_rel_stat_dn",
        f"weight_{electron_id_var}_eff_FixedThresh09_rel_sys_up",
        f"weight_{electron_id_var}_eff_FixedThresh09_rel_sys_dn",
        f"weight_{electron_id_var}_misid_pi_FixedThresh09",
        f"weight_{electron_id_var}_misid_pi_FixedThresh09_rel_stat_up",
        f"weight_{electron_id_var}_misid_pi_FixedThresh09_rel_stat_dn",
        f"weight_{electron_id_var}_misid_pi_FixedThresh09_rel_sys_up",
        f"weight_{electron_id_var}_misid_pi_FixedThresh09_rel_sys_dn",
        # NB: no K->l fake rates corrections (yet) for binary LID...
    ]

    variables_e += lid_e

    lid_mu = [
        muon_id_var,
        # The following aliases for LID weights are already set when creating the standard lepton list (see ma.stdLep).
        # You can decide to alias them to something else.
        f"weight_{muon_id_var}_eff_UniformEff90",
        f"weight_{muon_id_var}_eff_UniformEff90_rel_stat_up",
        f"weight_{muon_id_var}_eff_UniformEff90_rel_stat_dn",
        f"weight_{muon_id_var}_eff_UniformEff90_rel_sys_up",
        f"weight_{muon_id_var}_eff_UniformEff90_rel_sys_dn",
        f"weight_{muon_id_var}_misid_pi_UniformEff90",
        f"weight_{muon_id_var}_misid_pi_UniformEff90_rel_stat_up",
        f"weight_{muon_id_var}_misid_pi_UniformEff90_rel_stat_dn",
        f"weight_{muon_id_var}_misid_pi_UniformEff90_rel_sys_up",
        f"weight_{muon_id_var}_misid_pi_UniformEff90_rel_sys_dn",
        f"weight_{muon_id_var}_misid_K_UniformEff90",
        f"weight_{muon_id_var}_misid_K_UniformEff90_rel_stat_up",
        f"weight_{muon_id_var}_misid_K_UniformEff90_rel_stat_dn",
        f"weight_{muon_id_var}_misid_K_UniformEff90_rel_sys_up",
        f"weight_{muon_id_var}_misid_K_UniformEff90_rel_sys_dn",
    ]

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

    variables.printAliases()

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

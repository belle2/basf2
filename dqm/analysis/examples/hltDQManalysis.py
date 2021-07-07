##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import basf2

path = basf2.create_path()

path.add_module('DQMHistAnalysisInputRootFile', FileList="dqm_histo.root")
path.add_module('DQMHistAnalysisHLT', bhabhaName="accept_bhabha_all", columnMapping={
    "accept_hadron": "hadron",
    "accept_bhabha_all": "bhabha_all",
    "accept_mumutight": "mumutight",
    "accept_gamma_gamma": "gamma_gamma",
    "accept_radmumu": "radmumu",
    "accept_offip": "offip",
    "cosmic": "cosmic",
}, l1Histograms=["fff", "ffo", "lml0", "ffb", "fp"],
    retentionPerUnit=[
    "ge3_loose_tracks_inc_1_tight_not_ee2leg",
    "Elab_gt_0.5_plus_2_others_with_Elab_gt_0.18_plus_no_clust_with_Ecms_gt_2.0",
    "selectee",
    "Estargt2_GeV_cluster", ])
path.add_module('DQMHistAnalysisOutputFile', SaveHistos=False, SaveCanvases=True)

basf2.print_path(path)
basf2.process(path)

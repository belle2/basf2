import basf2

path = basf2.create_path()

path.add_module('DQMHistAnalysisInputRootFile', FileList="dqm_histo.root")
path.add_module('DQMHistAnalysisHLT', bhabhaName="accept_bhabha_all", columnMapping={
    "selectmumu": "selectmumu",
    "single_muon\\10": "single_muon\\10",
    "ECLMuonPair": "ECLMuonPair",
    "ge3_loose_tracks_inc_1_tight_not_ee2leg": "ge3_loose_tracks_inc_1_tight_not_ee2leg",
    "2_loose_tracks_0.8ltpstarmaxlt4.5_GeVc_not_ee2leg_ee1leg1trk_eexx":
    "2_loose_tracks_0.8ltpstarmaxlt4.5_GeVc_not_ee2leg_ee1leg1trk_eexx",
    "accept_hadron": "accept_hadron",
    "accept_bhabha_all": "accept_bhabha_all",
    "accept_mumu_2trk": "accept_mumu_2trk",
    "accept_mumutight": "accept_mumutight",
    "accept_gamma_gamma": "accept_gamma_gamma",
    "accept_radmumu": "accept_radmumu",
    "accept_offip": "accept_offip",
    "accept_tau_2trk": "accept_tau_2trk",
    "accept_tau_Ntrk": "accept_tau_Ntrk",
}, l1Histograms=["fff", "ffo", "lml0", "ffb", "fp"])
path.add_module('DQMHistAnalysisOutputFile', SaveHistos=False, SaveCanvases=True)

basf2.print_path(path)
basf2.process(path)

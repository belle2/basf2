import basf2

path = basf2.create_path()

path.add_module('DQMHistAnalysisInputRootFile', InputRootFile="dqm_e0007r003735.root", SelectFolders="softwaretrigger")
path.add_module('DQMHistAnalysisHLT', bhabhaName="accept_bhabha", columnMapping={
    "selectmumu": "selectmumu",
    "single_muon": "single_muon",
    "ECLMuonPair": "ECLMuonPair",
    "ge3_loose_tracks_inc_1_tight_not_ee2leg": "ge3_loose_tracks_inc_1_tight_not_ee2leg",
    "2_loose_tracks_0.8ltpstarmaxlt4.5_GeVc_not_ee2leg_ee1leg1trk_eexx":
    "2_loose_tracks_0.8ltpstarmaxlt4.5_GeVc_not_ee2leg_ee1leg1trk_eexx",
    "accept_hadron": "accept_hadron",
    "accept_bhabha": "accept_bhabha",
    "accept_mumu_1trk": "accept_mumu_1trk",
    "accept_mumu_2trk": "accept_mumu_2trk",
    "accept_gamma_gamma ": "accept_gamma_gamma ",
})
path.add_module('DQMHistAnalysisOutputFile', SaveHistos=False, SaveCanvases=True)

basf2.print_path(path)
basf2.process(path)

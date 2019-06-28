ECL_PHYSICS = [
    "filter 0.3ltEstar max clustlt2 GeV plus 2 others gt 0.2 GeV",
    "filter ge1 Estargt2 GeV neutral clst 2232 or 130145 not gg2clst ee2clst ee1leg eeBrem",
    "filter ge1 Estargt2 GeV neutral clst 32130 not gg2clst ee1leg1clst ee1leg1trk eeBrem",
]

ECL_PRESCALED = [
    "filter 1 electron Estargt1 GeV clust in 45115 and no other clust Estargt0.3 GeV",
    "filter 1 Estargt1 GeV cluster no other cluster Estargt0.3 GeV",
    "filter 1 photon Estargt1 GeV clust not low not 45115 no other clust Estargt0.3 GeV",
    "filter gg2clst",
    "filter ge1 Estargt2 GeV chrg clst 22145 not gg2clst ee2clst ee1leg",
    "filter ggEndcapLoose",
    "filter n2GeVPhotonBarrelge1",
    "filter n2GeVPhotonEndcapge1",
    "filter Estargt2 GeV cluster",
    "filter ECLMuonPair",
]

CDC_PHYSICS = [
    "filter ge3 loose tracks inc 1 tight not ee2leg",
    "filter 2 loose tracks 0.8ltpstarmaxlt4.5 GeVc",
    "filter 2 loose tracks 0.8ltpstarmaxlt4.5 GeVc not ee2leg ee1leg1trk eexx",
    "filter 2 loose tracks pstarmaxgt4.5 GeVc not ee2leg ee1leg1trk ee1leg1e eeBrem muonPairV",
]

CDC_PRESCALED = [
    "filter 2 loose tracks inc 1 tight q==0 pstarmaxlt0.8 GeVc not eexx",
    "filter 2 loose tracks pstarmaxgt4.5 GeVc",
    "filter 2 loose tracks pstarmaxlt0.8 GeVc",
    "filter ge1 tight track",
]

PHYSICS = [
    "filter 1 photon Estargt1 GeV clust in 45115 and no other clust Estargt0.3 GeV",
    "filter ggBarrelLoose"
]

QED = [
    "filter ee flat 0 19",
    "filter ee flat 19 22",
    "filter ee flat 22 25",
    "filter ee flat 25 30",
    "filter ee flat 30 35",
    "filter ee flat 35 45",
    "filter ee flat 45 60",
    "filter ee flat 60 90",
    "filter ee flat 90 180",
    "filter selectee",
    "filter selee1leg1clst",
    "filter selee1leg1trk",
    "filter eexxSelect",
    "filter radiative Bhabha",
    "filter selectmumu",
    "filter single muon",
]

LEVEL1 = [
    "filter L1 trigger",
    "filter passthrough l1 trigger delayed bhabha",
    "filter passthrough l1 trigger random",
    "filter dummy"
]

VETOES = [
    "filter ee1leg",
    "filter ee1leg1clst",
    "filter ee1leg1e",
    "filter ee1leg1trk",
    "filter ee2clst",
    "filter ee2leg",
    "filter eeBrem",
    "filter eexx",
    "filter muonPairV",
]

RESULTS = [
    "total events",
    "l1 decision",
    "final decision",
    "all total result",
    "filter total result",
    "skim total result"
]

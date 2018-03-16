"""
This file contains cuts used globally in the analysis.
Each cut is a python dictionary, containing:
      cut_name - internally used identifier,
      cut - the cut itself in root-compatible form
      cut_alias - nice tex-compatible version of cut name, which might contain the cut value. Some time
            we use ":::" to separate cut name from cut value.
If you want to modify selection cut, please modify cuts_Kpi0 list and its content.
"""

#: Dummy cut. Works as a template
no_cut = {"cut": "(1>0)", "cut_name": "NC", "cut_alias": "Rec-level"}


def cut_merger(cuts, cut_alias=""):
    """
    Function used to merge list of cuts. Frequently used in the package.
    """
    cutstring = "(1>0)"
    namestring = ""
    if isinstance(cuts, list):
        for cut in cuts:
            namestring += "_" + cut["cut_name"]
            cutstring += "&&" + cut["cut"]
    return {"cut": cutstring, "cut_name": namestring, "cut_alias": cut_alias}


#: Requirement of correct reconstruction and truth-mathcing for all signal particles
cut_MCTrue = {
    "cut_name": "SIGFS",
    "cut": "(B_MCTrue==1) && (B_K_isSignal==1) && (B_Kh0_isSignal==1) && (B_Kh1_isSignal==1) &&\
     (B_R_isSignal==1) && (B_Rh0_isSignal==1) && (B_Rh1_isSignal==1) && (B_gd0_isSignal==1) && (B_gd1_isSignal==1)"
}

#: Requirement of incorrect reconstruction or truth-mathcing for signal particles
cut_SxF = {
    "cut_name": "SxF",
    "cut": "((B_MCTrue==1) && (B_K_isSignal==1) && (B_Kh0_isSignal==1) && (B_Kh1_isSignal==1) &&\
     (B_R_isSignal==1) && (B_Rh0_isSignal==1) && (B_Rh1_isSignal==1) && (B_gd0_isSignal==1) && (B_gd1_isSignal==1)) == 0"
}

#: Cut on minimal photon energy
cut_mingE = {
    "cut_name": "mingE",
    "cut_alias": "$E^{\\text{min}}(\\gamma)$ ::: $> 0.1\\ GeV$",
    "cut": " ( B_g0_E > 0.1 ) && ( B_g1_E > 0.1)"
}

#: Cut on minimal pi0 energy
cut_minpi0E = {
    "cut_name": "cut_minpi0E",
    "cut_alias": "$E(\\pi^0)$ ::: $> 0.25\\ GeV$",
    "cut": " (B_Kh1_E > 0.25)"
}

#: Cut on minimal pio momentum
cut_minpi0p = {
    "cut_name": "cut_minpi0p",
    "cut_alias": "$p(\\pi^0)$ ::: $> 0.4\\ GeV/c$",
    "cut": " (B_Kh1_p>0.4)"
}

#: K*mass window
cut_MK = {
    "cut_name": "cut_MK",
    "cut_alias": "$M(K^*(892)$ ::: $\\in [0.792, 0.992]\\ GeV/c^2$",
    "cut": " (B_MK > 0.792) && (B_MK < 0.992)"
}

#: Rho mass window
cut_MR = {
    "cut_name": "cut_MR",
    "cut_alias": "$M(\\rho(770)$ ::: $\\in [0.52, 1.05]\\ GeV/c^2$",
    "cut": " (B_MR > 0.52) && (B_MR < 1.05)"
}

#: dE fit range
cut_dE = {
    "cut_name": "cut_dE",
    "cut_alias": "$\\Delta E$ ::: $\\in [-0.1, 0.1]\\ GeV/c^2$",
    "cut": " (B_deltaE_corr > -0.1) && (B_deltaE_corr < 0.1)"
}

#: Mbc fit range
cut_Mbc = {
    "cut_name": "cut_Mbc",
    "cut_alias": "$M_{bc}$ ::: $\\in [5.255, 5.289]\\ GeV/c^2$",
    "cut": "(B_Mbc_corr > 5.255) && (B_Mbc_corr < 5.289)"
}

#: K* helicity fit range
cut_hK = {
    "cut_name": "cut_hK",
    "cut_alias": "$K^*(892) hel.$ ::: $\\in [-1, 0.92]$",
    "cut": "(B_helK > -1.) && (B_helK < 0.92)"
}

#: Rho helicity fit range
cut_hR = {
    "cut_name": "cut_hR",
    "cut_alias": "$\\rho(770) hel.$ ::: $\\in [-0.95, 0.95]$",
    "cut": " (B_helR > -0.95) && (B_helR < 0.95)"
}

#: Best candidate requirement
cut_BC = {
    "cut_name": "BestCand",
    "cut_alias": "Vertex quality rank ::: 1",
    "cut": "B_vtx_rank == 1"
}

#: Combined pi0 cuts
cut_FitRange = cut_merger([cut_MK, cut_MR, cut_dE, cut_Mbc, cut_hK, cut_hR], "Fit ranges")

#: PID cut for charged hadrons
cut_pi0 = cut_merger([cut_mingE, cut_minpi0E, cut_minpi0p], "$\\pi^0$\\ cuts")

#: Combined fit range cuts
cut_FitRange = cut_merger([cut_MK, cut_MR, cut_dE, cut_Mbc, cut_hK, cut_hR], "Fit ranges")

#: PID cut for charged hadrons
cut_chhPID = {
    "cut_name": "PID",
    "cut_alias": "PID of charged",
    "cut": "(max(B_Rh0_PID_K_vs_pi,B_Rh1_PID_K_vs_pi)<0.4) && ((B_Kh0_PID_K_vs_pi > 0.6) || (B_Kh1_PID_K_vs_pi < 0.4))"
}

#: List of selection cuts
cuts_Kpi0 = [cut_FitRange, cut_pi0, cut_BC]

#: Combined selection cuts
cut_Kpi0 = cut_merger(cuts_Kpi0, "DummySel")

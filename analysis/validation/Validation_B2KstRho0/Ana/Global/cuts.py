"""
This file contains cuts used globally in the analysis.
Each cut is a python dictionary, containing:
      cut_name - internally used identifier,
      cut - the cut itself in root-compatible form
      cut_alias - nice tex-compatible version of cut name, which might contain the cut value. Some time
            we use ":::" to separate cut name from cut value.
If you want to modify selection cut, please modify cuts_Kpi0 list and its content.
"""

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

cut_MCTrue = {
    "cut_name": "SIGFS",
    "cut": "(B__MCTrue==1) && (B__K_isSignal==1) && (B__Kh0_isSignal==1) && (B__Kh1_isSignal==1) &&\
     (B__R_isSignal==1) && (B__Rh0_isSignal==1) && (B__Rh1_isSignal==1) && (B__gd0_isSignal==1) && (B__gd1_isSignal==1)"
}

cut_SxF = {
    "cut_name": "SxF",
    "cut": "((B__MCTrue==1) && (B__K_isSignal==1) && (B__Kh0_isSignal==1) && (B__Kh1_isSignal==1) &&\
     (B__R_isSignal==1) && (B__Rh0_isSignal==1) && (B__Rh1_isSignal==1) && (B__gd0_isSignal==1) && (B__gd1_isSignal==1)) == 0"
}

cut_mingE = {
    "cut_name": "mingE",
    "cut_alias": "$E^{\\text{min}}(\\gamma)$ ::: $> 0.1\\ GeV$",
    "cut": " ( B__g0_E > 0.1 ) && ( B__g1_E > 0.1)"
}

cut_minpi0E = {
    "cut_name": "cut_minpi0E",
    "cut_alias": "$E(\\pi^0)$ ::: $> 0.25\\ GeV$",
    "cut": " (B__Kh1_E > 0.25)"
}

cut_minpi0p = {
    "cut_name": "cut_minpi0p",
    "cut_alias": "$p(\\pi^0)$ ::: $> 0.4\\ GeV/c$",
    "cut": " (B__Kh1_p>0.4)"
}


cut_MK = {
    "cut_name": "cut_MK",
    "cut_alias": "$M(K^*(892)$ ::: $\\in [0.792, 0.992]\\ GeV/c^2$",
    "cut": " (B__MK > 0.792) && (B__MK < 0.992)"
}

cut_MR = {
    "cut_name": "cut_MR",
    "cut_alias": "$M(\\rho(770)$ ::: $\\in [0.52, 1.05]\\ GeV/c^2$",
    "cut": " (B__MR > 0.52) && (B__MR < 1.05)"
}

cut_dE = {
    "cut_name": "cut_dE",
    "cut_alias": "$\\Delta E$ ::: $\\in [-0.1, 0.1]\\ GeV/c^2$",
    "cut": " (B__deltaE_corr > -0.1) && (B__deltaE_corr < 0.1)"
}

cut_Mbc = {
    "cut_name": "cut_Mbc",
    "cut_alias": "$M_{bc}$ ::: $\\in [5.255, 5.289]\\ GeV/c^2$",
    "cut": "(B__Mbc_corr > 5.255) && (B__Mbc_corr < 5.289)"
}

cut_hK = {
    "cut_name": "cut_hK",
    "cut_alias": "$K^*(892) hel.$ ::: $\\in [-1, 0.92]$",
    "cut": "(B__helK > -1.) && (B__helK < 0.92)"
}

cut_hR = {
    "cut_name": "cut_hR",
    "cut_alias": "$\\rho(770) hel.$ ::: $\\in [-0.95, 0.95]$",
    "cut": " (B__helR > -0.95) && (B__helR < 0.95)"
}

cut_BC = {
    "cut_name": "BestCand",
    "cut_alias": "Vertex quality rank ::: 1",
    "cut": "B__vtx_rank == 1"
}

cut_pi0 = cut_merger([cut_mingE, cut_minpi0E, cut_minpi0p], "$\\pi^0$\\ cuts")

cut_FitRange = cut_merger([cut_MK, cut_MR, cut_dE, cut_Mbc, cut_hK, cut_hR], "Fit ranges")

cut_chhPID = {
    "cut_name": "PID",
    "cut_alias": "PID of charged",
    "cut": "(max(B__Rh0_PID_K_vs_pi,B__Rh1_PID_K_vs_pi)<0.4) && ((B__Kh0_PID_K_vs_pi > 0.6) || (B__Kh1_PID_K_vs_pi < 0.4))"
}

cuts_Kpi0 = [cut_FitRange, cut_pi0, cut_BC]
cut_Kpi0 = cut_merger(cuts_Kpi0, "DummySel")

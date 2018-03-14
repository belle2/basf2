from Plotters import *
Variables = [fit_variables]


def Sig_CutEffect(mode, cut=no_cut):
    """
    This function compares distribution of variables defined in gloabl Variables variable
    at generation, reconstruction and selection levels.
    """
    Samples = [
        {"file_name": root_storage + mode + ".root",
         "MCTrue": False,
         "tree_name": "MCB2KstRho",
         "cut": no_cut,
         "alias": "Generated"
         },
        {"file_name": root_storage + mode + ".root",
         "MCTrue": False,
         "tree_name": "B2KstRho",
         "cut": cut_MCTrue,
         "alias": "Reconstructed"
         },
        {"file_name": root_storage + mode + ".root",
         "MCTrue": False,
         "tree_name": "B2KstRho",
         "cut": cut_merger([cut_MCTrue, cut]),
         "alias":"Selected"
         }
    ]

    Description = mode + "_CutEffect"
    Compare(Samples, Variables, Description, extra="")


def Sig_RecSel(mode, cut=no_cut):
    """
    This function compares distribution of variables defined in global Variables variable
    at reconstruction and selection levels.
    """
    Samples = [
        {"file_name": root_storage + mode + ".root",
         "MCTrue": False,
         "tree_name": "B2KstRho",
         "cut": cut_MCTrue,
         "alias": "Reconstructed"
         },
        {"file_name": root_storage + mode + ".root",
         "MCTrue": False,
         "tree_name": "B2KstRho",
         "cut": cut_merger([cut_MCTrue, cut]),
         "alias":"Selected"
         }
    ]

    Description = mode + "_CutEffect"
    Compare(Samples, Variables, Description, extra="")


def Sig_SxF(mode, cut=no_cut):
    """
    This function compares distribution of variables defined in global Variables variable
    for signal and self cross-feed candidates.
    """
    Samples = [
        {"file_name": root_storage + mode + ".root",
         "MCTrue": False,
         "tree_name": "B2KstRho",
         "cut": cut_merger([cut_MCTrue, cut]),
         "alias":"Signal"
         },
        {"file_name": root_storage + mode + ".root",
         "MCTrue": False,
         "tree_name": "B2KstRho",
         "cut": cut_merger([cut_SxF, cut]),
         "alias":"SxF"
         }
    ]

    # Currently obsolete, but you may also add fraction of SxF to the plot.
    Extra = {
        "expression": "1%0",
        "yaxis": "f_{SXF}"
    }

    Description = mode + "_CutEffect"
    Compare(Samples, Variables, Description, extra="")


if __name__ == "__main__":
    Variables = [fit_variables]
    for mode in ['Bu_Rho0Kst+_K+pi0_Longitudinal', 'Bu_Rho0Kst+_K+pi0_Transverse']:
        Sig_CutEffect(mode, cut_Kpi0)

    mode = 'Bu_Rho0Kst+_K+pi0'
    Variables = [pi0_variables]
    Sig_CutEffect(mode, cut_Kpi0)

    Variables = [CB_suppression, {'Mpi0Rec': pi0_variables['Mpi0Rec']}]
    Sig_RecSel(mode, cut_Kpi0)

    Variables = [event_variables]
    Sig_SxF(mode, cut_merger([cut_FitRange, cut_pi0]))

    file_name = root_storage + mode + ".root"
    tree_name = "B2KstRho"
    cut = cut_MCTrue
    for variable in fit_variables.values():
        Resolution_Plots(file_name, tree_name, variable, alias="", cut=cut, p_address="Plots/", h_address="Hists/")

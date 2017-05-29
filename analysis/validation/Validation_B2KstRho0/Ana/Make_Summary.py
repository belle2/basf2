import ROOT
from ROOT import TFile, TCanvas, TH2F, TF2
from ROOT import gROOT, gStyle
import os
import sys
import inspect
currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
parentdir = os.path.dirname(currentdir)
sys.path.insert(0, parentdir)
from Global.fit_variables import fit_variables
from Global.selection_variables import *
from Global.cuts import *
from Global.paths import root_storage
from Decorations.Plotters import eff_err
import math


def print_to_file(tab, file_addr, caption="", label=""):
    f = open(file_addr, 'w')
    print("\\begin{table}[h!]", file=f)
    print("\\centering", file=f)
    print("\\begin{tabular}{l|" + (len(tab[0]) - 1) * "c" + "}", file=f)
    print("&".join(tab[0]) + "\\\\", file=f)
    print("\\hline", file=f)
    for l in tab[1:]:
        print("&".join(l) + "\\\\", file=f)
    print("\\end{tabular}", file=f)
    if label:
        print("\\label{" + label + "}", file=f)
    print("\\caption{" + caption + "}", file=f)
    print("\\end{table}\n", file=f)
    return True


def round_uncert(err):
    if err < 0:
        print("Negative error: " + str(err) + ", don't know what to do with it")
        return False

    if err > 1:
        err_pow = int(math.log10(abs(err)))
        return [round(err, -err_pow), err_pow]
    else:
        err_pow = 0
        while int(err) < 2:
            err_pow += 1
            err *= 10
        return [round(err, 0) / pow(10, err_pow), err_pow]


def frmt_val_err(valerr, err=0):
    if type(valerr) == list:
        val = valerr[0]
        err = valerr[1]
    else:
        val = valerr
    if err == 0:
        return "$" + str(val) + "$"
    n_digits = round_uncert(err)[1]
    errstr = str(round_uncert(err)[0])
    valstr = str(round(val, n_digits))
    return "$" + valstr + " \\pm " + errstr + "$"


def Rec_Eff_Table():
    Kpi0_root_trans = TFile(root_storage + "Bu_Rho0Kst+_K+pi0_Transverse.root")
    Kpi0_tree_trans_gen = Kpi0_root_trans.Get("MCB2KstRho")
    Kpi0_N_trans_gen = Kpi0_tree_trans_gen.GetEntries()

    Kpi0_tree_trans_rec = Kpi0_root_trans.Get("B2KstRho")
    Kpi0_N_trans_rec = Kpi0_tree_trans_rec.GetEntries()
    Kpi0_Nmc_trans_rec = Kpi0_tree_trans_rec.GetEntries(cut_MCTrue['cut'])
    Kpi0_Nsxf_trans_rec = Kpi0_tree_trans_rec.GetEntries(cut_SxF['cut'])
    Kpi0_Nmc_trans_sel = Kpi0_tree_trans_rec.GetEntries(cut_merger([cut_Kpi0, cut_MCTrue])['cut'])
    Kpi0_Nsxf_trans_sel = Kpi0_tree_trans_rec.GetEntries(cut_merger([cut_Kpi0, cut_SxF])['cut'])
    Kpi0_N_trans_sel = Kpi0_Nmc_trans_sel + Kpi0_Nsxf_trans_sel

    Kpi0_root_long = TFile(root_storage + "Bu_Rho0Kst+_K+pi0_Longitudinal.root")
    Kpi0_tree_long_gen = Kpi0_root_long.Get("MCB2KstRho")
    Kpi0_N_long_gen = Kpi0_tree_long_gen.GetEntries()

    Kpi0_tree_long_rec = Kpi0_root_long.Get("B2KstRho")
    Kpi0_N_long_rec = Kpi0_tree_long_rec.GetEntries()
    Kpi0_Nmc_long_rec = Kpi0_tree_long_rec.GetEntries(cut_MCTrue['cut'])
    Kpi0_Nsxf_long_rec = Kpi0_tree_long_rec.GetEntries(cut_SxF['cut'])
    Kpi0_Nmc_long_sel = Kpi0_tree_long_rec.GetEntries(cut_merger([cut_Kpi0, cut_MCTrue])['cut'])
    Kpi0_Nsxf_long_sel = Kpi0_tree_long_rec.GetEntries(cut_merger([cut_Kpi0, cut_SxF])['cut'])
    Kpi0_N_long_sel = Kpi0_Nmc_long_sel + Kpi0_Nsxf_long_sel

    Kpi0_eff_trans = eff_err(Kpi0_Nmc_trans_rec, Kpi0_N_trans_gen)[0]
    Kpi0_sxf_trans = eff_err(Kpi0_Nsxf_trans_rec, Kpi0_N_trans_rec)[0]
    Kpi0_efferr_trans = eff_err(Kpi0_Nmc_trans_rec, Kpi0_N_trans_gen)[1]
    Kpi0_sxferr_trans = eff_err(Kpi0_Nsxf_trans_rec, Kpi0_N_trans_rec)[1]
    Kpi0_eff_long = eff_err(Kpi0_Nmc_long_rec, Kpi0_N_long_gen)[0]
    Kpi0_sxf_long = eff_err(Kpi0_Nsxf_long_rec, Kpi0_N_long_rec)[0]
    Kpi0_efferr_long = eff_err(Kpi0_Nmc_long_rec, Kpi0_N_long_gen)[1]
    Kpi0_sxferr_long = eff_err(Kpi0_Nsxf_long_rec, Kpi0_N_long_rec)[1]

    Kpi0_sel_eff_trans = eff_err(Kpi0_Nmc_trans_sel, Kpi0_N_trans_gen)[0]
    Kpi0_sel_sxf_trans = eff_err(Kpi0_Nsxf_trans_sel, Kpi0_N_trans_sel)[0]
    Kpi0_sel_efferr_trans = eff_err(Kpi0_Nmc_trans_sel, Kpi0_N_trans_gen)[1]
    Kpi0_sel_sxferr_trans = eff_err(Kpi0_Nsxf_trans_sel, Kpi0_N_trans_sel)[1]
    Kpi0_sel_eff_long = eff_err(Kpi0_Nmc_long_sel, Kpi0_N_long_gen)[0]
    Kpi0_sel_sxf_long = eff_err(Kpi0_Nsxf_long_sel, Kpi0_N_long_sel)[0]
    Kpi0_sel_efferr_long = eff_err(Kpi0_Nmc_long_sel, Kpi0_N_long_gen)[1]
    Kpi0_sel_sxferr_long = eff_err(Kpi0_Nsxf_long_sel, Kpi0_N_long_sel)[1]

    tab = []
    width = 30

    tab.append(['{:{width}}'.format(" ", width=width),
                '{:{align}{width}}'.format("Transverse", align='^', width=width),
                '{:{align}{width}}'.format("Longitdinal", align='^', width=width)])
    tab.append(["\\hline"])

    tab.append(['{:{align}{width}}'.format("$N^{\\text{Gen.}}$", align='^', width=width),
                '{:{align}{width}}'.format(str(Kpi0_N_trans_gen), align='^', width=width),
                '{:{align}{width}}'.format(str(Kpi0_N_long_gen), align='^', width=width)])
    tab.append(["\\hline"])
    tab.append(['{:{align}{width}}'.format("$N^{\\text{Rec.}}_{\\text{MC True}}$", align='^', width=width),
                '{:{align}{width}}'.format(str(Kpi0_Nmc_trans_rec), align='^', width=width),
                '{:{align}{width}}'.format(str(Kpi0_Nmc_long_rec), align='^', width=width)])
    tab.append(['{:{align}{width}}'.format("$N^{\\text{Rec.}}_{\\text{SxF}}$", align='^', width=width),
                '{:{align}{width}}'.format(str(Kpi0_Nsxf_trans_rec), align='^', width=width),
                '{:{align}{width}}'.format(str(Kpi0_Nsxf_long_rec), align='^', width=width)])
    tab.append(['{:{align}{width}}'.format("$\\epsilon^{\\text{Rec.}}$", align='^', width=width),
                '{:{align}{width}}'.format(frmt_val_err(Kpi0_eff_trans, Kpi0_efferr_trans), align='^', width=width),
                '{:{align}{width}}'.format(frmt_val_err(Kpi0_eff_long, Kpi0_efferr_long), align='^', width=width)])
    tab.append(['{:{align}{width}}'.format("$f^{\\text{Rec.}}_{\\text{SXF}}$",
                                           align='^',
                                           width=width),
                '{:{align}{width}}'.format("$" + str(round(eff_err(Kpi0_Nsxf_trans_rec,
                                                                   Kpi0_Nmc_trans_rec + Kpi0_Nsxf_trans_rec)[0] * 100,
                                                           0)) + "\\%$",
                                           align='^',
                                           width=width),
                '{:{align}{width}}'.format("$" + str(round(eff_err(Kpi0_Nsxf_long_rec,
                                                                   Kpi0_Nmc_long_rec + Kpi0_Nsxf_long_rec)[0] * 100,
                                                           0)) + "\\%$",
                                           align='^',
                                           width=width)])
    tab.append(["\\hline"])
    tab.append(['{:{align}{width}}'.format("$N^{\\text{Sel.}}_{\\text{MC True}}$", align='^', width=width),
                '{:{align}{width}}'.format(str(Kpi0_Nmc_trans_sel), align='^', width=width),
                '{:{align}{width}}'.format(str(Kpi0_Nmc_long_sel), align='^', width=width)])
    tab.append(['{:{align}{width}}'.format("$N^{\\text{Sel.}}_{\\text{SxF}}$", align='^', width=width),
                '{:{align}{width}}'.format(str(Kpi0_Nsxf_trans_sel), align='^', width=width),
                '{:{align}{width}}'.format(str(Kpi0_Nsxf_long_sel), align='^', width=width)])
    tab.append(['{:{align}{width}}'.format("$\\epsilon^{\\text{Rec.}}\\times\\epsilon^{\\text{Sel.}}$", align='^', width=width),
                '{:{align}{width}}'.format(frmt_val_err(Kpi0_sel_eff_trans, Kpi0_sel_efferr_trans), align='^', width=width),
                '{:{align}{width}}'.format(frmt_val_err(Kpi0_sel_eff_long, Kpi0_sel_efferr_long), align='^', width=width)])
    tab.append(['{:{align}{width}}'.format("$f^{\\text{Rec.+Sel.}}_{\\text{SXF}}$",
                                           align='^',
                                           width=width),
                '{:{align}{width}}'.format("$" + str(round(eff_err(Kpi0_Nsxf_trans_sel,
                                                                   Kpi0_Nmc_trans_sel + Kpi0_Nsxf_trans_sel)[0] * 100,
                                                           0)) + "\\%$",
                                           align='^',
                                           width=width),
                '{:{align}{width}}'.format("$" + str(round(eff_err(Kpi0_Nsxf_long_sel,
                                                                   Kpi0_Nmc_long_sel + Kpi0_Nsxf_long_sel)[0] * 100,
                                                           0)) + "\\%$",
                                           align='^',
                                           width=width)])

    os.system("mkdir -p Report/")
    print_to_file(
        tab,
        "Report/Tab_Rec_Summary.tex",
        caption="Efficiencies of recontruction and fraction of the cross-feed candidates.",
        label="tab:Summary")
    return


def Cut_Table(cuts):

    width = 30
    tab = []
    tab.append(['{:{width}}'.format("Variable", width=width),
                '{:{align}{width}}'.format("Cut", align='^', width=width)])

    for c in cuts:
        print(c["cut_alias"])
        tab.append(['{:{width}}'.format(c["cut_alias"].split(":::")[0], width=width),
                    '{:{width}}'.format(c["cut_alias"].split(":::")[1], width=width)])

    os.system("mkdir -p Report/")
    print_to_file(tab, "Report/Tab_Selection.tex", caption="Cuts applied for selection in validation study", label="tab:Selection")
    return


def Sel_Eff_Table_Cuts_vs_Samples(mode, cuts, name):
    root_trans = TFile(root_storage + mode + "_Transverse.root")
    tree_trans_rec = root_trans.Get("B2KstRho")
    root_long = TFile(root_storage + mode + "_Longitudinal.root")
    tree_long_rec = root_long.Get("B2KstRho")

    N_rec_trans = tree_trans_rec.GetEntries()
    N_rec_trans_true = tree_trans_rec.GetEntries(cut_MCTrue["cut"])
    N_rec_trans_sxf = N_rec_trans - N_rec_trans_true
    N_rec_long = tree_long_rec.GetEntries()
    N_rec_long_true = tree_long_rec.GetEntries(cut_MCTrue["cut"])
    N_rec_long_sxf = N_rec_long - N_rec_long_true

    width = 30
    title_line = ['{:{width}}'.format("Samples", width=width)]
    for c in cuts:
        title_line.append('{:{width}}'.format(c["cut_alias"].split(":::")[0], width=width))
    tab = [title_line]

    line_T = ['$\\epsilon_{\\text{Signal}}^{\\text{Transverse}}$']
    line_L = ['$\\epsilon_{\\text{Signal}}^{\\text{Longitdinal}}$']
    line_SL = ['$\\epsilon_{\\text{SxF}}^{\\text{Longitdinal}}$']
    line_SLF = ['$f_{\\text{SxF}}^{\\text{Longitdinal}}$']
    line_ST = ['$\\epsilon_{\\text{SxF}}^{\\text{Transverse}}$']
    line_STF = ['$f_{\\text{SxF}}^{\\text{Transverse}}$']

    line_S = ['Self cross-feed']
    line_SF = ['(fraction)']

    for c in cuts:
        sel_trans = tree_trans_rec.GetEntries(cut_merger([c, cut_MCTrue])["cut"])
        sel_long = tree_long_rec.GetEntries(cut_merger([c, cut_MCTrue])["cut"])
        line_T.append('{:{align}{width}}'.format(frmt_val_err(eff_err(sel_trans, N_rec_trans_true)), align='^', width=width))
        line_L.append('{:{align}{width}}'.format(frmt_val_err(eff_err(sel_long, N_rec_long_true)), align='^', width=width))
        sel_sxf_L = tree_long_rec.GetEntries(cut_merger([c, cut_SxF])["cut"])
        sel_sxf_T = tree_trans_rec.GetEntries(cut_merger([c, cut_SxF])["cut"])
        sel_sxf = sel_sxf_L + sel_sxf_T
        line_S.append(
            '{:{align}{width}}'.format(
                frmt_val_err(
                    eff_err(
                        sel_sxf,
                        N_rec_trans_sxf +
                        N_rec_long_sxf)),
                align='^',
                width=width))
        line_SF.append('{:{align}{width}}'.format(
            "$" + str(round(eff_err(sel_sxf, sel_sxf + sel_trans + sel_long)[0] * 100, 0)) + "\\%$", align='^', width=width))
        line_SL.append('{:{align}{width}}'.format(frmt_val_err(eff_err(sel_sxf_L, N_rec_long_sxf)), align='^', width=width))
        line_SLF.append('{:{align}{width}}'.format(
            "$" + str(round(eff_err(sel_sxf_L, sel_sxf_L + sel_long)[0] * 100, 0)) + "\\%$", align='^', width=width))
        line_ST.append('{:{align}{width}}'.format(frmt_val_err(eff_err(sel_sxf_T, N_rec_trans_sxf)), align='^', width=width))
        line_STF.append('{:{align}{width}}'.format(
            "$" + str(round(eff_err(sel_sxf_T, sel_sxf_T + sel_trans)[0] * 100, 0)) + "\\%$", align='^', width=width))

    tab.append(line_L)
    tab.append(line_SL)
    tab.append(line_SLF)
    tab.append(["\\hline"])
    tab.append(line_T)
    tab.append(line_ST)
    tab.append(line_STF)

    os.system("mkdir -p Report/")
    print_to_file(
        tab,
        "Report/Tab_Eff_Summary.tex",
        caption="Detailed signal and self cross-feed selection efficiencies\
         for $B^+\\to\\rho^0K^{*+}(\\to K^+\\pi^0)$\\ candidates.",
        label="tab:EffDetailed")
    return


if __name__ == "__main__":
    # If you change cuts, please make sure you indicate them here.
    Cut_Table([cut_mingE, cut_minpi0E, cut_minpi0p, cut_MK, cut_MR, cut_dE, cut_Mbc, cut_hK, cut_hR, cut_BC])
    Rec_Eff_Table()
    Sel_Eff_Table_Cuts_vs_Samples("Bu_Rho0Kst+_K+pi0", cuts_Kpi0 + [cut_merger(cuts_Kpi0, "Total")], "SelectionEfficiency")

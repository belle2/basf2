#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
    <input>MCvalidation.root</input>
    <output>SplitMultiplicityPlots.root</output>
    <contact>Frank Meier; frank.meier@belle2.org</contact>
    <description>Comparing generated kaon multiplicities, optionally split by charge and originating B meson flavor</description>
</header>
"""

import ROOT


def PlottingHistos(particle, pos, neg):
    ''' Plotting function'''

    nbins = int(2*range_dic[particle])

    hist = rdf_fix.Histo1D((particle, particle, nbins, -range_dic[particle], range_dic[particle]), pos)
    hist_neg = rdf_fix.Histo1D((f"{particle}_neg", f"{particle}_neg", nbins, -range_dic[particle], range_dic[particle]), neg)
    hist.Add(hist_neg.GetPtr())
    hist.SetTitle(f";{axis_dic[particle]}; Events")
    hist.GetListOfFunctions().Add(ROOT.TNamed('Description', f'{axis_dic[particle]} multiplicity'))
    hist.GetListOfFunctions().Add(ROOT.TNamed('Check', 'Shape should not change drastically.'))
    hist.GetListOfFunctions().Add(ROOT.TNamed('Contact', 'frank.meier@belle2.org'))
    hist.GetListOfFunctions().Add(ROOT.TNamed('MetaOptions', 'nostats'))
    hist.Write()
    # c1.Clear()


if __name__ == '__main__':

    # load in the root files
    rdf = ROOT.RDataFrame("Split", "MCvalidation.root")
    rdf_fix = rdf.Define("gen_Kn", "-gen_Km").Define("gen_K0bar", "-gen_antiK0")

    B = 'charged'

    axis_dic = {'Kpm': 'K^{+} / K^{#minus} from both B',
                'K0': 'K^{0} / #bar{K}^{0} from both B'
                }

    range_dic = {'Kpm': 4.5,
                 'K0': 3.5,
                 'Kpm_same': 4.5,
                 'Kpm_diff': 4.5,
                 'K0_same': 3.5,
                 'K0_diff': 3.5
                 }

    outputFile = ROOT.TFile("SplitMultiplicityPlots.root", "RECREATE")
    ROOT.gROOT.SetBatch(True)
    ROOT.gROOT.SetStyle("BELLE2")
    ROOT.gROOT.ForceStyle()

    PlottingHistos("Kpm", "gen_Kp", "gen_Kn")
    PlottingHistos("K0", "gen_K0", "gen_K0bar")
    if B == "charged":
        axis_dic['Kpm_same'] = 'K^{+} from B^{+} / K^{#minus} from B^{#minus}'
        axis_dic['Kpm_diff'] = 'K^{+} from B^{#minus} / K^{#minus} from B^{+}'
        axis_dic['K0_same'] = 'K^{0} from B^{+} / #bar{K}^{0} from B^{#minus}'
        axis_dic['K0_diff'] = 'K^{0} from B^{#minus} / #bar{K}^{0} from B^{+}'

        rdf_fix = rdf.Define("gen_Kp_Bn", "-gen_Kp_Bm")\
                     .Define("gen_Km_Bn", "-gen_Km_Bm")\
                     .Define("gen_K0_Bn", "-gen_K0_Bm")\
                     .Define("gen_antiK0_Bn", "-gen_antiK0_Bm")

        PlottingHistos("Kpm_same", "gen_Kp_Bp", "gen_Km_Bn")
        PlottingHistos("Kpm_diff", "gen_Kp_Bn", "gen_Km_Bp")
        PlottingHistos("K0_same", "gen_K0_Bp", "gen_antiK0_Bn")
        PlottingHistos("K0_diff", "gen_K0_Bn", "gen_antiK0_Bp")
    else:
        axis_dic['Kpm_same'] = 'K^{+} from B^{0} / K^{#minus} from #bar{B}^{0}'
        axis_dic['Kpm_diff'] = 'K^{+} from #bar{B}^{0} / K^{#minus} from B^{0}'
        axis_dic['K0_same'] = 'K^{0} from B^{0} / #bar{K}^{0} from #bar{B}^{0}'
        axis_dic['K0_diff'] = 'K^{0} from #bar{B}^{0} / #bar{K}^{0} from B^{0}'

        rdf_fix = rdf.Define("gen_Kp_B0bar", "-gen_Kp_antiB0")\
                     .Define("gen_Km_B0bar", "-gen_Km_antiB0")\
                     .Define("gen_K0_B0bar", "-gen_K0_antiB0")\
                     .Define("gen_antiK0_B0bar", "-gen_antiK0_antiB0")

        PlottingHistos("Kpm_same", "gen_Kp_B0", "gen_Km_B0bar")
        PlottingHistos("Kpm_diff", "gen_Kp_B0bar", "gen_Km_B0")
        PlottingHistos("K0_same", "gen_K0_B0", "gen_antiK0_B0bar")
        PlottingHistos("K0_diff", "gen_K0_B0bar", "gen_antiK0_B0")

    outputFile.Close()

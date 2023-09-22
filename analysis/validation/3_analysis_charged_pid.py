#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <input>"../PartGunChargedStableGenSim.root"</input>
  <output>ChargedPID_Validation.root</output>
  <contact>Sviatoslav Bilokin (sviatoslav.bilokin@desy.de)</contact>
  <description>
  Check the PID efficiency and fake rates
  </description>
</header>
"""

import basf2
import ROOT
from modularAnalysis import cutAndCopyList, inputMdst, fillParticleList, variablesToHistogram, printList
from reconstruction import add_reconstruction


INPUT_FILENAME = "../PartGunChargedStableGenSim.root"
OUTPUT_FILENAME = "ChargedPID_Validation.root"

global_id_names = ['electronID', 'muonID', 'pionID', 'kaonID', 'protonID']
particle_types = ['e', 'mu', 'pi', 'K', 'p']
particle_pdgs = [11, 13, 211, 321, 2212]

thresholds = [0.2, 0.8]
kin_variable_names = ['p', 'cosTheta']
kin_variable_bins = [(25, 0.2, 5), (10, -1, 1)]


def run_b2analysis():
    """
    Function to produce the validation ntuples via basf2.
    """
    main = basf2.Path()
    inputMdst(INPUT_FILENAME, path=main)
    add_reconstruction(path=main)
    track_quality_cuts = 'isSignal > 0'

    for pid, particle in zip(global_id_names, particle_types):
        fillParticleList(f'{particle}+:total', track_quality_cuts, path=main)
        printList(f'{particle}+:total', False, path=main)
        # Efficiency cuts:
        for cut_val in thresholds:
            cutAndCopyList(f'{particle}+:cut0{int(cut_val*10)}', f'{particle}+:total',
                           f'{pid} > {cut_val}', path=main)
            printList(f'{particle}+:cut0{int(cut_val*10)}', False, path=main)
            for var, bin_tuple in zip(kin_variable_names, kin_variable_bins):
                variablesToHistogram(f'{particle}+:cut0{int(cut_val*10)}',
                                     (var, *bin_tuple),
                                     filename=OUTPUT_FILENAME,
                                     path=main,
                                     directory=f'cut0{int(cut_val*10)}_{particle}_{pid}_{var}')
            # Fake rate cuts:
            for fake_pid in global_id_names:
                if fake_pid == pid:
                    continue
                cutAndCopyList(f'{particle}+:fake_{fake_pid}_cut0{int(cut_val*10)}', f'{particle}+:total',
                               f'{fake_pid} > {cut_val}', path=main)
                printList(f'{particle}+:fake_{fake_pid}_cut0{int(cut_val*10)}', False, path=main)
                for var, bin_tuple in zip(kin_variable_names, kin_variable_bins):
                    variablesToHistogram(f'{particle}+:fake_{fake_pid}_cut0{int(cut_val*10)}',
                                         (var, *bin_tuple),
                                         filename=OUTPUT_FILENAME,
                                         path=main,
                                         directory=f'fake_cut0{int(cut_val*10)}_{particle}_{fake_pid}_{var}')
        # Total rates:
        for var, bin_tuple in zip(kin_variable_names, kin_variable_bins):
            variablesToHistogram(f'{particle}+:total',
                                 (var, *bin_tuple),
                                 filename=OUTPUT_FILENAME,
                                 path=main,
                                 directory=f'total_{particle}_{var}')

    main.add_module('Progress')
    basf2.process(main)
    print(basf2.statistics)


def add_plots():
    """
    Adds TEfficiency objects to the produced ntuple.
    """
    root_file = ROOT.TFile(OUTPUT_FILENAME, "UPDATE")
    for pid, particle in zip(global_id_names, particle_types):
        for var in kin_variable_names:
            print(f'total_{particle}_{var}')
            total_obs_th1 = root_file.Get(f'total_{particle}_{var}').Get(var)
            for cut_val in thresholds:
                # print(f'cut0{int(cut_val*10)}_{particle}_{var}')
                eff_cut_obs_th1 = root_file.Get(f'cut0{int(cut_val*10)}_{particle}_{pid}_{var}').Get(var)
                eff_cut_obs_th1.Print()
                teff = ROOT.TEfficiency(eff_cut_obs_th1, total_obs_th1)
                teff.SetName(f'cut0{int(cut_val*10)}_{particle}_{pid}_{var}_eff')
                teff.SetTitle(f'Efficiency for {pid} > {cut_val} for {particle}; {var}; Efficiency')
                teff.GetListOfFunctions().Add(
                    ROOT.TNamed("Description", f"Efficiency plot of {pid} for {particle}")
                )
                teff.GetListOfFunctions().Add(ROOT.TNamed("Check", "Efficiency should not decrease"))
                teff.GetListOfFunctions().Add(ROOT.TNamed("Contact", "sviatoslav.bilokin@desy.de"))
                options = ""
                if cut_val == thresholds[-1]:
                    options = "shifter"
                teff.GetListOfFunctions().Add(ROOT.TNamed("MetaOptions", options))
                teff.Write()
                for fake_pid in global_id_names:
                    if fake_pid == pid:
                        continue
                    print(f'fake_cut0{int(cut_val*10)}_{particle}_{fake_pid}_{var}')
                    fake_cut_obs_th1 = root_file.Get(f'fake_cut0{int(cut_val*10)}_{particle}_{fake_pid}_{var}').Get(var)
                    fake_cut_obs_th1.Print()
                    tfake = ROOT.TEfficiency(fake_cut_obs_th1, total_obs_th1)
                    tfake.SetName(f'cut0{int(cut_val*10)}_{particle}_{fake_pid}_{var}_fake')
                    tfake.SetTitle(f'Fake rate for {fake_pid} > {cut_val} for {particle}; {var}; Fake rate')
                    tfake.GetListOfFunctions().Add(
                        ROOT.TNamed("Description", f"Fake rate plot of {fake_pid} for {particle}")
                    )
                    tfake.GetListOfFunctions().Add(ROOT.TNamed("Check", "Fake rates should not increase"))
                    tfake.GetListOfFunctions().Add(ROOT.TNamed("Contact", "sviatoslav.bilokin@desy.de"))
                    tfake.GetListOfFunctions().Add(ROOT.TNamed("MetaOptions", ""))
                    tfake.Write()
    root_file.Write()
    root_file.Close()


run_b2analysis()
add_plots()

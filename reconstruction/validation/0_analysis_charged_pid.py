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
  <input>PartGunChargedStableGenSim.root</input>
  <output>ChargedPID_Validation.root,Detector_ChargedPID_Validation.root</output>
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
from variables import variables

INPUT_FILENAME = "../PartGunChargedStableGenSim.root"
GLOBALID_OUTPUT_FILENAME = "Global_ChargedPID_Validation.root"
DET_PID_OUTPUT_FILENAME = "Detector_ChargedPID_Validation.root"

GLOBAL_ID_NAMES = ['electronID', 'muonID', 'pionID', 'kaonID', 'protonID']
PARTICLE_TYPES = ['e', 'mu', 'pi', 'K', 'p']
PARTICLE_PDGS = [11, 13, 211, 321, 2212]
DETECTORS = ['SVD', 'CDC', 'TOP', 'ARICH', 'ECL', 'KLM']
DETECTOR_EXPERTS = ["sviatoslav.bilokin@desy.de",  # SVD
                    "sviatoslav.bilokin@desy.de",  # CDC
                    "sviatoslav.bilokin@desy.de",  # TOP
                    "sviatoslav.bilokin@desy.de",  # ARICH
                    "sviatoslav.bilokin@desy.de",  # ECL
                    "sviatoslav.bilokin@desy.de"  # KLM
                    ]

PID_EXPERTS = ["sviatoslav.bilokin@desy.de",  # e
               "sviatoslav.bilokin@desy.de",  # mu
               "sviatoslav.bilokin@desy.de",  # pi
               "sviatoslav.bilokin@desy.de",  # K
               "sviatoslav.bilokin@desy.de"  # p
               ]

THRESHOLDS = [0.2, 0.8]
KIN_VAR_NAMES = ['p', 'cosTheta']
KIN_VAR_BINS = [(25, 0.2, 5), (10, -1, 1)]
DEFAULT_OPTIONS = "pvalue-warn=0.5"


def get_expert(pid: str, for_detector: bool = False):
    """
    Returns the email of expert for detector or PID.
    """
    tup = (PARTICLE_TYPES, PID_EXPERTS)
    if for_detector:
        tup = (DETECTORS, DETECTOR_EXPERTS)
    for det_or_part, exp in zip(*tup):
        if for_detector:
            if f'_{det_or_part}' in pid:
                return exp
        elif pid.startswith(det_or_part.lower()):
            return exp
    return "sviatoslav.bilokin@desy.de"


def add_aliases():
    """
    Returns aliases for the detector probability variables
    """
    result = []
    for pdg, name in zip(PARTICLE_PDGS, PARTICLE_TYPES):
        for det in DETECTORS:
            new_name = f'pidProbabilityExpert_{name}_{det}'
            variables.addAlias(new_name, f'pidProbabilityExpert({pdg},{det})')
            result += [new_name]
    return result


DETECTOR_PIDS = add_aliases()
print(DETECTOR_PIDS)


def run_b2analysis():
    """
    Function to produce the validation ntuples via basf2.
    """
    main = basf2.Path()
    inputMdst('default', INPUT_FILENAME, path=main)
    add_reconstruction(path=main)
    track_quality_cuts = 'isSignal > 0'

    for pid, particle in zip(GLOBAL_ID_NAMES, PARTICLE_TYPES):
        fillParticleList(f'{particle}+:total', track_quality_cuts, path=main)
        printList(f'{particle}+:total', False, path=main)
        # Efficiency cuts:
        for cut_val in THRESHOLDS:
            cutAndCopyList(f'{particle}+:cut0{int(cut_val*10)}', f'{particle}+:total',
                           f'{pid} > {cut_val}', path=main)
            printList(f'{particle}+:cut0{int(cut_val*10)}', False, path=main)
            for var, bin_tuple in zip(KIN_VAR_NAMES, KIN_VAR_BINS):
                variablesToHistogram(f'{particle}+:cut0{int(cut_val*10)}',
                                     (var, *bin_tuple),
                                     filename=GLOBALID_OUTPUT_FILENAME,
                                     path=main,
                                     directory=f'cut0{int(cut_val*10)}_{particle}_{pid}_{var}')
            # Fake rate cuts:
            for fake_pid in GLOBAL_ID_NAMES:
                if fake_pid == pid:
                    continue
                cutAndCopyList(f'{particle}+:fake_{fake_pid}_cut0{int(cut_val*10)}', f'{particle}+:total',
                               f'{fake_pid} > {cut_val}', path=main)
                printList(f'{particle}+:fake_{fake_pid}_cut0{int(cut_val*10)}', False, path=main)
                for var, bin_tuple in zip(KIN_VAR_NAMES, KIN_VAR_BINS):
                    variablesToHistogram(f'{particle}+:fake_{fake_pid}_cut0{int(cut_val*10)}',
                                         (var, *bin_tuple),
                                         filename=GLOBALID_OUTPUT_FILENAME,
                                         path=main,
                                         directory=f'fake_cut0{int(cut_val*10)}_{particle}_{fake_pid}_{var}')

        # Run detector based selection only for the lower threshold and the momentum bins:
        det_cut_value = THRESHOLDS[0]
        for det_pid_var in DETECTOR_PIDS:
            # Efficiency only
            if f'_{particle}_' in det_pid_var:
                cutAndCopyList(f'{particle}+:eff_{det_pid_var}_cut0{int(det_cut_value*10)}', f'{particle}+:total',
                               f'{det_pid_var} > {det_cut_value}', path=main)
                variablesToHistogram(f'{particle}+:eff_{det_pid_var}_cut0{int(det_cut_value*10)}',
                                     (KIN_VAR_NAMES[0], *KIN_VAR_BINS[0]),
                                     filename=DET_PID_OUTPUT_FILENAME,
                                     path=main,
                                     directory=f'eff_cut0{int(det_cut_value*10)}_{particle}_{det_pid_var}_{KIN_VAR_NAMES[0]}')
        # Total rates:
        for var, bin_tuple in zip(KIN_VAR_NAMES, KIN_VAR_BINS):
            variablesToHistogram(f'{particle}+:total',
                                 (var, *bin_tuple),
                                 filename=GLOBALID_OUTPUT_FILENAME,
                                 path=main,
                                 directory=f'total_{particle}_{var}')
        variablesToHistogram(f'{particle}+:total',
                             (KIN_VAR_NAMES[0], *KIN_VAR_BINS[0]),
                             filename=DET_PID_OUTPUT_FILENAME,
                             path=main,
                             directory=f'total_{particle}_{KIN_VAR_NAMES[0]}')

    main.add_module('Progress')
    basf2.process(main)
    print(basf2.statistics)


def add_global_plots():
    """
    Adds TEfficiency objects to the produced ntuple for global PID.
    """
    root_file = ROOT.TFile(GLOBALID_OUTPUT_FILENAME, "UPDATE")
    for pid, particle in zip(GLOBAL_ID_NAMES, PARTICLE_TYPES):
        for var in KIN_VAR_NAMES:
            print(f'total_{particle}_{var}')
            total_obs_th1 = root_file.Get(f'total_{particle}_{var}').Get(var)
            for cut_val in THRESHOLDS:
                eff_cut_obs_th1 = root_file.Get(f'cut0{int(cut_val*10)}_{particle}_{pid}_{var}').Get(var)
                eff_cut_obs_th1.Print()
                teff = ROOT.TEfficiency(eff_cut_obs_th1, total_obs_th1)
                teff.SetName(f'cut0{int(cut_val*10)}_{particle}_{pid}_{var}_eff')
                teff.SetTitle(f'Efficiency for {pid} > {cut_val} for {particle}; {var}; Efficiency')
                teff.GetListOfFunctions().Add(
                    ROOT.TNamed("Description", f"Efficiency plot of {pid} for {particle}")
                )
                teff.GetListOfFunctions().Add(ROOT.TNamed("Check", "Efficiency should not decrease"))
                teff.GetListOfFunctions().Add(ROOT.TNamed("Contact", get_expert(pid)))
                options = DEFAULT_OPTIONS
                if cut_val == THRESHOLDS[-1]:
                    options += " ,shifter"
                teff.GetListOfFunctions().Add(ROOT.TNamed("MetaOptions", options))
                teff.Write()
                for fake_pid in GLOBAL_ID_NAMES:
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
                    tfake.GetListOfFunctions().Add(ROOT.TNamed("Contact", get_expert(pid)))
                    tfake.GetListOfFunctions().Add(ROOT.TNamed("MetaOptions", DEFAULT_OPTIONS))
                    tfake.Write()
    ROOT.TNamed(
        "Description",
        "Global Charged PID validation plots. Shifter plots are the log-likelihood PID efficiency plots \
        for e, μ, π, K and p particles in bins of momentum and cosTheta. If any degradation of efficiency \
        is spotted here, please take a look at the expert plots. A decrease of efficiency might indicate: \
        a) a work to improve the data/MC agreement; b) possible decrease of fake rate levels, \
        see the expert plots in this section; \
        c) a problem with a sub-detector likelihood, please take a look at \
        the expert section of detector-only PID section. ").Write()
    root_file.Write()
    root_file.Close()


def add_detector_plots():
    """
    Adds TEfficiency objects to the produced ntuple for detector PID.
    """
    root_file = ROOT.TFile(DET_PID_OUTPUT_FILENAME, "UPDATE")
    var = KIN_VAR_NAMES[0]
    cut_val = THRESHOLDS[0]
    for det_pid_var in DETECTOR_PIDS:
        for particle in PARTICLE_TYPES:
            total_obs_th1 = root_file.Get(f'total_{particle}_{var}').Get(var)
            if f'_{particle}_' in det_pid_var:
                eff_cut_obs_th1 = root_file.Get(f'eff_cut0{int(cut_val*10)}_{particle}_{det_pid_var}_{var}').Get(var)
                teff = ROOT.TEfficiency(eff_cut_obs_th1, total_obs_th1)
                teff.SetName(f'cut0{int(cut_val*10)}_{particle}_{det_pid_var}_{var}_eff')
                teff.SetTitle(f'Efficiency for {det_pid_var} > {cut_val} for {particle}; {var}; Efficiency')
                teff.GetListOfFunctions().Add(
                    ROOT.TNamed("Description", f"Efficiency plot of {det_pid_var} for {particle}")
                )
                teff.GetListOfFunctions().Add(ROOT.TNamed("Check", "Efficiency should not decrease"))
                teff.GetListOfFunctions().Add(ROOT.TNamed("Contact", get_expert(det_pid_var, for_detector=True)))
                # Only for experts:
                teff.GetListOfFunctions().Add(ROOT.TNamed("MetaOptions", DEFAULT_OPTIONS))
                teff.Write()
    ROOT.TNamed(
        "Description",
        "Detector-based Charged PID validation."
    ).Write()

    root_file.Write()
    root_file.Close()


run_b2analysis()
add_global_plots()
add_detector_plots()

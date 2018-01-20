"""
Script to be called by gridcontrol to simulate events with the given channel.
"""

from glob import glob

import basf2
import os

import generators
from softwaretrigger.path_functions import RAW_SAVE_STORE_ARRAYS, DEFAULT_HLT_COMPONENTS
from background import get_background_files

from rawdata import add_packers
from simulation import add_simulation


def add_generation(path, event_class):
    """
    Add the different generators to the path (can be changed by the event_class input parameter).
    :param path: The path to add the generator to.
    :param event_class: The channel to simulate.
    """
    # background only
    if event_class == "background_only":
        pass

    # 4 lepton final states
    elif event_class == "eeee":
        generators.add_aafh_generator(path, finalstate="e+e-e+e-", preselection=False)
    elif event_class == "eemumu":
        generators.add_aafh_generator(path, finalstate="e+e-mu+mu-", preselection=False)

    # Tau events
    elif event_class == "tau":
        generators.add_kkmc_generator(path, finalstate="tau+tau-")

    # TODO:
    # elif event_class == "tau_to_1_prong_1_prong":
    #     add_kkgen_input(path, input_file='tau.input.dat', tau_decay_table_name="1_prong_1_prong",
    #                     use_preselection=True)
    # elif event_class == "tau_to_e_gamma":
    #     add_kkgen_input(path, input_file='tau.input.dat', tau_decay_table_name="1_prong_e_gamma",
    #                     use_preselection=True, n_photons_min=1)
    # elif event_class == "tau_to_mu_gamma":
    #     add_kkgen_input(path, input_file='tau.input.dat', tau_decay_table_name="1_prong_mu_gamma",
    #                     use_preselection=True, n_photons_min=1)
    # elif event_class == "tau_to_e_nu":
    #     add_kkgen_input(path, input_file='tau.input.dat', tau_decay_table_name="1_prong_e_nu",
    #                     use_preselection=True)
    # elif event_class == "tau_to_mu_nu":
    #     add_kkgen_input(path, input_file='tau.input.dat', tau_decay_table_name="1_prong_mu_nu",
    #                     use_preselection=True)

    # 2 lepton final states, Bhabha
    elif event_class == "mumu":
        generators.add_phokhara_generator(path, finalstate="mu+mu-")
    elif event_class == "mumu_kkgen":
        generators.add_kkmc_generator(path, finalstate="mu+mu-")
    elif event_class == "ee":
        generators.add_babayaganlo_generator(path, finalstate="ee")
    elif event_class == "gg":
        generators.add_babayaganlo_generator(path, finalstate="gg")

    # Continuum
    elif event_class == "continuum_ccbar":
        generators.add_continuum_generator(path, finalstate="ccbar")
    elif event_class == "continuum_uubar":
        generators.add_continuum_generator(path, finalstate="uubar")
    elif event_class == "continuum_ddbar":
        generators.add_continuum_generator(path, finalstate="ddbar")
    elif event_class == "continuum_ssbar":
        generators.add_continuum_generator(path, finalstate="ssbar")

    # "Signal" B decays
    elif event_class == "BB_charged":
        generators.add_evtgen_generator(path, finalstate="charged")
    elif event_class == "BB_mixed":
        generators.add_evtgen_generator(path, finalstate="mixed")
    # TODO:
    # elif event_class == "B_to_nu_nu":
    #     add_evtgen_input(path, userDECFile='B_to_nu_nu')
    # elif event_class == "B_to_pi0_pi0":
    #     add_evtgen_input(path, userDECFile='B_to_pi0_pi0')
    # elif event_class == "B_to_rho0_gamma":
    #     add_evtgen_input(path, userDECFile='B_to_rho0_gamma')
    # elif event_class == "B_to_JpsiKS_e_e":
    #     add_evtgen_input(path, userDECFile='B_to_JpsiKS_e_e')

    # pi pi
    elif event_class == "pipi":
        generators.add_phokhara_generator(path, finalstate="pi+pi-")
    elif event_class == "pipipi":
        generators.add_phokhara_generator(path, finalstate="pi+pi-pi0")

    # Fail for everything else
    else:
        raise NotImplementedError()


def main():
    # Get all parameters for this calculation
    channel = os.environ.get("channel")
    output_file = os.environ.get("output_file")
    random_seed = os.environ.get("random_seed")
    n_events = int(os.environ.get("n_events"))
    phase = int(os.environ.get("phase"))

    print("Parameters: ")
    print("channel:", channel)
    print("output_file:", output_file)
    print("random_seed:", random_seed)
    print("n_events:", n_events)
    print("phase:", n_events)

    log_file = output_file.replace(".root", ".log")

    # Do the real calculation
    basf2.set_random_seed(random_seed)

    path = basf2.create_path()

    runNumber = 0
    if phase == 2:
        runNumber = 1002

    path.add_module("EventInfoSetter", evtNumList=[n_events], runList=runNumber)

    if phase == 2:
        path.add_module('Gearbox', fileName="geometry/Beast2_phase2.xml")
    else:
        path.add_module("Gearbox")
    path.add_module("Geometry")

    add_generation(path, event_class=channel)

    # We do not want to have PXD data reduction in the simulation - as this is not performed in the real detector at
    # at this stage
    add_simulation(path, usePXDDataReduction=False, bkgfiles=get_background_files())

    add_packers(path, components=DEFAULT_HLT_COMPONENTS)

    # We are adding the PXDDigits here on purpose, as they will be in the final data (stored on tape)
    path.add_module("RootOutput",
                    branchNames=["EventMetaData", "PXDDigits"] + RAW_SAVE_STORE_ARRAYS,
                    outputFileName=output_file)

    basf2.log_to_file(log_file)
    basf2.print_path(path)
    basf2.process(path)


if __name__ == "__main__":
    main()

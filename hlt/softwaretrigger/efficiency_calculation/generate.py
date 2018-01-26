"""
Script to be called by gridcontrol to simulate events with the given channel.
"""

from glob import glob

import basf2
import os

import generators
from softwaretrigger.path_functions import RAW_SAVE_STORE_ARRAYS, ALWAYS_SAVE_REGEX, DEFAULT_HLT_COMPONENTS
from background import get_background_files

from rawdata import add_packers
from simulation import add_simulation

from L1trigger import add_tsim


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
        generators.add_aafh_generator(path, finalstate='e+e-e+e-', minmass=0.1,
                                      subweights=[1.000e+00, 1.384e+02, 1.574e+07, 3.018e+07,
                                                  1.000e+00, 1.293e+00, 2.418e+00, 1.131e+00],
                                      maxsubweight=2.0, maxfinalweight=6.0)
    elif event_class == "eemumu":
        generators.add_aafh_generator(path, finalstate='e+e-mu+mu-', minmass=0.1,
                                      subweights=[1.000e+00, 1.234e+01, 1.160e+04, 2.906e+04,
                                                  1.000e+00, 1.123e+00, 3.892e+00, 4.169e+00],
                                      maxsubweight=2.0, maxfinalweight=1.5)

    # Tau events
    elif event_class == "tau":
        generators.add_kkmc_generator(path, finalstate="tau+tau-")

    # 2 lepton final states, Bhabha
    elif event_class == "mumu":
        generators.add_kkmc_generator(path, finalstate="mu+mu-")
    elif event_class == "ee":
        generators.add_bhwide_generator(path, minangle=0.5)
    elif event_class == "gg":
        generators.add_babayaganlo_generator(path, finalstate="gg", minangle=0.0, minenergy=0.01)

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

    expNumber = 0
    if phase == 2:
        expNumber = 1002

    path.add_module("EventInfoSetter", evtNumList=[n_events], expList=expNumber)

    if phase == 2:
        path.add_module('Gearbox', fileName="geometry/Beast2_phase2.xml")
    else:
        path.add_module("Gearbox")
    path.add_module("Geometry")

    add_generation(path, event_class=channel)

    # We do not want to have PXD data reduction in the simulation - as this is not performed in the real detector at
    # at this stage
    add_simulation(path, usePXDDataReduction=False, bkgfiles=get_background_files())

    add_tsim(path, Belle2Phase="Phase{}".format(phase), PrintResult=True)

    add_packers(path, components=DEFAULT_HLT_COMPONENTS)

    # We are adding the PXDDigits here on purpose, as they will be in the final data (stored on tape)
    path.add_module("RootOutput",
                    branchNames=["PXDDigits"] + RAW_SAVE_STORE_ARRAYS + ALWAYS_SAVE_REGEX,
                    outputFileName=output_file)

    basf2.log_to_file(log_file)
    basf2.print_path(path)
    basf2.process(path)


if __name__ == "__main__":
    main()

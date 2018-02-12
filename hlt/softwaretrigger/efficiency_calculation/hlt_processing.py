"""
Script to be called by gridcontrol to reconstruct already simulated events.
"""

import basf2
import os

from softwaretrigger.path_functions import add_softwaretrigger_reconstruction, add_hlt_processing, DEFAULT_HLT_COMPONENTS, \
    RAW_SAVE_STORE_ARRAYS

from rawdata import add_unpackers, add_packers
from simulation import add_roiFinder


def main():
    """Reconstruct the already generated events and store the results to disk"""
    # Get all parameters for this calculation
    input_file_list = os.environ.get("input_file_list")
    phase = int(os.environ.get("phase"))
    roi_filter = bool(os.environ.get("roi_filter"))
    hlt_mode = os.environ.get("hlt_mode")

    input_file_list = input_file_list.split("#")

    print("input_file_list:", input_file_list)
    print("phase:", phase)

    # Now start the real basf2 calculation
    path = basf2.create_path()
    path.add_module("RootInput", inputFileNames=input_file_list)

    if hlt_mode == "collision_filter":
        add_hlt_processing(path, run_type="collision", softwaretrigger_mode="hlt_filter")
    elif hlt_mode == "collision_monitor":
        add_hlt_processing(path, run_type="collision", softwaretrigger_mode="monitor")
    elif hlt_mode == "cosmics_monitor":
        add_hlt_processing(path, run_type="cosmics", softwaretrigger_mode="monitor", data_taking_period="phase{}".format(phase))
    else:
        basf2.B2FATAL("hlt_mode {} not supported".format(hlt_mode))

    basf2.print_path(path)
    basf2.process(path)


if __name__ == "__main__":
    main()

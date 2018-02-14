"""
Script to be called by gridcontrol to reconstruct already simulated events.
"""

import basf2
import os

from softwaretrigger.path_functions import add_softwaretrigger_reconstruction, add_hlt_processing, DEFAULT_HLT_COMPONENTS, \
    RAW_SAVE_STORE_ARRAYS

from rawdata import add_unpackers, add_packers
from validation import statistics_plots, event_timing_plot


def main():
    """Reconstruct the already generated events and store the results to disk"""
    # Get all parameters for this calculation
    input_file_list = os.environ.get("input_file_list")
    phase = int(os.environ.get("phase"))
    hlt_mode = os.environ.get("hlt_mode")
    mem_statistics_file = os.environ.get("mem_statistics_file")
    vmem_mem_statistics_file = mem_statistics_file.replace("_memory.root", "_vmem.root")
    rss_mem_statistics_file = mem_statistics_file.replace("_memory.root", "_rss.root")
    dqm_histogram_file = mem_statistics_file.replace("_memory.root", "_dqm.root")
    rss_mem_statistics_file = mem_statistics_file.replace("_memory.root", "_rss.root")

    input_file_list = input_file_list.split("#")

    print("input_file_list:", input_file_list)
    print("phase:", phase)

    # Now start the real basf2 calculation
    path = basf2.create_path()
    path.add_module("RootInput", inputFileNames=input_file_list)

    path.add_module("HistoManager", histoFileName=dqm_histogram_file)

    if hlt_mode == "collision_filter":
        add_hlt_processing(path, run_type="collision", softwaretrigger_mode="hlt_filter")
    elif hlt_mode == "collision_monitor":
        add_hlt_processing(path, run_type="collision", softwaretrigger_mode="monitoring")
    elif hlt_mode == "cosmics_monitor":
        add_hlt_processing(path, run_type="cosmics", softwaretrigger_mode="monitoring", data_taking_period="phase{}".format(phase))
    else:
        basf2.B2FATAL("hlt_mode {} not supported".format(hlt_mode))

    # memory profile
    path.add_module('Profile', outputFileName=vmem_mem_statistics_file, rssOutputFileName=rss_mem_statistics_file)

    basf2.print_path(path)
    basf2.process(path)

    # Print call statistics
    print(basf2.statistics)

    # statistics_plots('EvtGenSimRec_statistics.root', contact='Thomas.Hauth@kit.edu',
    #                 jobDesc='a standard simulation and reconstruction job with generic EvtGen events',
    #                 prefix='EvtGenSimRec')
    # event_timing_plot('../EvtGenSimRec.root', 'EvtGenSimRec_statistics.root', contact='tkuhr',
    #                  jobDesc='a standard simulation and reconstruction job with generic EvtGen events',
    #                  prefix='EvtGenSimRec')


if __name__ == "__main__":
    main()

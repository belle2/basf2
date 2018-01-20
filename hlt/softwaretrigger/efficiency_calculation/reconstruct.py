"""
Script to be called by gridcontrol to reconstruct already simulated events.
"""

import basf2
import os

from L1trigger import add_tsim
from softwaretrigger.path_functions import add_softwaretrigger_reconstruction, DEFAULT_HLT_COMPONENTS, \
    RAW_SAVE_STORE_ARRAYS

from rawdata import add_unpackers, add_packers
from simulation import add_roiFinder


def main():
    # Get all parameters for this calculation
    input_file = os.environ.get("input_file")
    output_file = os.environ.get("output_file")
    phase = int(os.environ.get("phase"))

    print("input_file:", input_file)
    print("output_file:", output_file)
    print("phase:", phase)

    raw_output_file = output_file.replace(".root", "_raw.root")

    log_file = output_file.replace(".root", ".log")

    # Now start the real basf2 calculation
    path = basf2.create_path()
    path.add_module("RootInput", inputFileName=input_file)

    if phase == 2:
        path.add_module("Gearbox", fileName="geometry/Beast2_phase2.xml")
    else:
        path.add_module("Gearbox")

    add_unpackers(path, components=DEFAULT_HLT_COMPONENTS)

    add_tsim(path, Belle2Phase="Phase{}".format(phase))

    # Add the ST and also write out all variables connected to it. Also, do not cut, but just write out the variables
    add_softwaretrigger_reconstruction(path, store_array_debug_prescale=1, softwaretrigger_mode="monitoring")

    # TODO: until the ROI finding HLT setup is handled properly, we have to do this "manually" here
    add_roiFinder(path, reco_tracks="RecoTracks")
    path.add_module('PXDdigiFilter', PXDDigitsInsideROIName='PXDDigitsInsideROI', ROIidsName='ROIs')
    add_packers(path, components=["PXD"])

    path.add_module("RootOutput", outputFileName=output_file,
                    branchNames=["EventMetaData", "SoftwareTriggerResult", "SoftwareTriggerVariables", "TRGSummary"])

    raw_save_store_arrays_without_rois = RAW_SAVE_STORE_ARRAYS
    raw_save_store_arrays_without_rois.pop(raw_save_store_arrays_without_rois.index("ROIs"))
    raw_save_store_arrays_without_rois.append("RawPXDs")

    path.add_module("RootOutput", outputFileName=raw_output_file,
                    branchNames=["EventMetaData", "SoftwareTriggerResult", "TRGSummary"] + raw_save_store_arrays_without_rois)

    basf2.log_to_file(log_file)
    basf2.print_path(path)
    basf2.process(path)


if __name__ == "__main__":
    main()

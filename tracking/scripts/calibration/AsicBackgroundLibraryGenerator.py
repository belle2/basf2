##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
import sys
import rawdata
import tracking

"""
        Sample script to generate ASIC cross-talk library.  Usage:

        basf2 AsicBackgroundLibraryGenerator.py <cosmic_raw_data_file> <output_asicbg_root_file> <data_type>

          cosmic_raw_data_file : input raw data files

          output_asicbg_root_file: output root files

          data_type: 'mc' or 'data'
    """


def main():

    GT = ['patch_main_release-07'][0]
    basf2.B2WARNING(f'Current global tag is {GT}, please update the proper global tag to match the data')
    basf2.conditions.prepend_globaltag(GT)
    path = basf2.create_path()
    path.add_module("Progress")

    if len(sys.argv) != 4:
        sys.exit("Three arguments are required: input_root, output_root, and data_type")
    # Input raw data file
    inputFilename = sys.argv[1]
    # Output file
    file_name = sys.argv[2]
    # data or mc
    sample_name = sys.argv[3]
    print(inputFilename, '\n', file_name, '\n', sample_name)

    cdc_type = {'data': 'RawCDCs', 'mc': 'CDCHits'}
    branches = ['EventMetaData',  cdc_type[f'{sample_name}']]
    path.add_module("RootInput", inputFileNames=inputFilename, branchNames=branches)
    path.add_module("Gearbox")
    path.add_module("Geometry", useDB=True)

    if sample_name == 'data':
        rawdata.add_unpackers(path, components=['CDC'])

    tracking.add_track_finding(path, components=['CDC'])

    path.add_module("AsicBackgroundLibraryCreator",
                    inputTracks="CDCTrackVector",
                    inputWireHits="CDCWireHitVector",
                    AsicLibraryFileName=file_name,
                    #                  maximalDistanceSignal=100.2,
                    #                  minimalDistanceBackground=0.0,
                    #                  useAxialHitsOnly=False,
                    #                  minimalHitsOnTrack=0,
                    writeExtraVars=True
                    )

    basf2.print_path(path)

    # Run the execution path
    basf2.process(path)


if __name__ == "__main__":
    main()

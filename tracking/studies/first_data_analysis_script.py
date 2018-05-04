#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os

import basf2
import rawdata

from tracking import add_hit_preparation_modules, add_track_finding, add_track_fit_and_track_creator
from tracking.harvesting_validation import recorded_data_module


def get_output_file_name(file_name):
    input_file_names = os.environ["FILE_NAMES"].split()
    assert len(input_file_names) == 1
    input_file_name = input_file_names[0]
    input_file_name = os.path.splitext(os.path.basename(input_file_name))[0]

    return input_file_name + "_" + file_name


if __name__ == "__main__":
    basf2.reset_database()
    basf2.use_central_database(os.environ["BASF2_GDT"])

    path = basf2.create_path()

    input_file_names = os.environ["FILE_NAMES"].split()
    print(f"Using input files {input_file_names}")

    path.add_module("RootInput", inputFileNames=input_file_names)

    path.add_module("Gearbox")
    path.add_module("Geometry", useDB=True)

    rawdata.add_unpackers(path)

    add_hit_preparation_modules(path)
    path.add_module('SetupGenfitExtrapolation', energyLossBrems=False, noiseBrems=False)

    add_track_finding(path, svd_ckf_mode="VXDTF2_before_with_second_ckf", prune_temporary_tracks=False)

    add_track_fit_and_track_creator(path)

    # Resetting MinClusterTime parameter to turn of cut on cluster time
    for m in path.modules():
        if m.name() == "SVDSpacePointCreator":
            m.param("MinClusterTime", -999)

    path.add_module("RootOutput", outputFileName=get_output_file_name("reconstructed.root"))

    path.add_module(recorded_data_module.TrackInfoHarvester(output_file_name=get_output_file_name('trackLevelInformation.root')))
    path.add_module(recorded_data_module.HitInfoHarvester(output_file_name=get_output_file_name('hitLevelInformation.root')))
    path.add_module(recorded_data_module.EventInfoHarvester(output_file_name=get_output_file_name('eventLevelInformation.root')))

    basf2.print_path(path)
    basf2.process(path)
    print(basf2.statistics)

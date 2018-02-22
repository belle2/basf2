#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Script to generate grid-control config file to try the hlt reconstruction on a set of raw input files
"""

import basf2
import os
import shutil
from ROOT import Belle2

import gridcontrol_helper
import argparse
import subprocess


def main():
    """Reconstruct the already generated events and store the results to disk"""

    parser = argparse.ArgumentParser()
    parser.add_argument("hlt_script", help="Name of the hlt scripts to excute, for example phase2_cosmics_reco_monitor.py")
    parser.add_argument("raw_folder_location", help="Folder where the raw input files are located")
    parser.add_argument("global_tag", help="Global tag to download and use")
    parser.add_argument("--local", help="Execute on the local system and not via batch processing",
                        action="store_true", default=False)
    parser.add_argument("--ignore-db-failure", help="Ignore if the DB download was not successful (expert option)",
                        action="store_true", default=False)

    args = parser.parse_args()

    script_name = os.path.basename(args.hlt_script).replace(".py", "")
    # this encodes if its expressreco or hlt, because the files are named similar but are
    # in different folders on disk
    script_name_folder = os.path.basename(os.path.dirname(args.hlt_script))

    working_folder = "hlt_raw_processing_{}_{}".format(script_name_folder, script_name)
    print("Using working folder {}".format(working_folder))

    if os.path.exists(working_folder):
        shutil.rmtree(working_folder)
    os.mkdir(working_folder)

    abs_working_folder = os.path.abspath(working_folder)
    abs_db_target = os.path.join(abs_working_folder, "local_db")
    hlt_steering_file = Belle2.FileSystem.findFile(args.hlt_script)

    gc_config = gridcontrol_helper.write_gridcontrol_hlt_test(working_folder=abs_working_folder,
                                                              hlt_steering_file=hlt_steering_file,
                                                              dataset_folder=args.raw_folder_location,
                                                              local_db_path=os.path.join(abs_db_target, "database.txt"),
                                                              local_execution=args.local)

    # download DB
    try:
        subprocess.check_call(["conditionsdb", "download", "-c", "--exclude", "FEI", "-j10", args.global_tag,
                               abs_db_target])
    except subprocess.CalledProcessError as e:
        print("DB Download failed.")
        if not args.ignore_db_failure:
            raise e

    print("Grid control config file {} created".format(gc_config))
    gridcontrol_helper.call_gridcontrol(gc_config, retries=0)


if __name__ == "__main__":
    main()

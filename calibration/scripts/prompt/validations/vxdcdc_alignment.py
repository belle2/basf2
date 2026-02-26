#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
from glob import glob
from pathlib import Path
import argparse

import matplotlib
import matplotlib.pyplot as plt

from prompt import ValidationSettings

import alignment_validation.dimuon as dimuonval
import alignment_validation.cosmics as cosmicval

import ROOT as r
r.PyConfig.IgnoreCommandLineOptions = True
r.gROOT.SetBatch()

matplotlib.use('Agg')
plt.style.use("belle2")

#: Tells the automated system some details of this script
settings = ValidationSettings(name="vxdcdc_alignment",
                              description=__doc__,
                              download_files=[],
                              expert_config=None)


def run_validation(job_path, input_data_path=None, **kwargs):
    '''job_path will be replaced with path/to/calibration_results
    input_data_path will be replaced with path/to/data_path used for calibration
    e.g. /group/belle2/dataprod/Data/PromptSkim/'''

    collector_output_dir_cosmic = Path(job_path) / 'VXDCDCalignment_validation/0/collector_output/cosmic/'
    collector_output_dir_mumu = Path(job_path) / 'VXDCDCalignment_validation/0/collector_output/mumu/'

    output_dir = Path(kwargs.get('output_dir', 'VXDCDCAlignmentValidation_output'))
    # create output directory if it does not exist
    output_dir.mkdir(parents=True, exist_ok=True)

    pattern_cosmic = str(collector_output_dir_cosmic) + "/*/cosmic_ana.root"
    pattern_mumu = str(collector_output_dir_mumu) + "/*/dimuon_ana.root"

    def hadd_and_get_merged_file(filenames_pattern, input_type):
        root_files = glob(filenames_pattern)
        merged_file = output_dir / f"{input_type}.root"

        if len(root_files) > 1:
            os.system(f"hadd -f {merged_file} {' '.join(root_files)}")
        elif len(root_files) == 1:
            os.system(f"cp {root_files[0]} {merged_file}")
        else:
            raise FileNotFoundError(f"No root files found for pattern: {filenames_pattern}")

        return str(merged_file)

    cosmic_file = hadd_and_get_merged_file(pattern_cosmic, "cosmics")
    mumu_file = hadd_and_get_merged_file(pattern_mumu, "dimuon")

    print(f"Merged ntuples saved in {cosmic_file} and {mumu_file}")
    print("Running validation...")

    cosmicval.run_validation([cosmic_file], output_dir=str(output_dir / "cosmics/"))
    dimuonval.run_validation([mumu_file], output_dir=str(output_dir / "dimuon/"))

    # Now merge std histograms stored in ColectorOutput.root
    histo_pattern_cosmic = str(collector_output_dir_cosmic) + "/*/CollectorOutput.root"
    histo_pattern_mumu = str(collector_output_dir_mumu) + "/*/CollectorOutput.root"

    histo_cosmic = hadd_and_get_merged_file(histo_pattern_cosmic, "cosmic_CollectorOutput")
    histo_mumu = hadd_and_get_merged_file(histo_pattern_mumu, "dimuon_CollectorOutput")

    print(f"Merged CollectorOutput histograms saved in {histo_cosmic} and {histo_mumu}")

    print("Alignment validation completed.")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawTextHelpFormatter)

    # b2val-prompt-run wants to pass to the script also input_data_path
    # and requested_iov. As they are not required by this validation I just accept
    # them together with calibration_results_dir and then ignore them
    parser.add_argument('calibration_results_dir',
                        help='The directory that contains the collector outputs',
                        nargs='+')

    parser.add_argument('-o', '--output_dir',
                        help='The directory where all the output will be saved',
                        default='VXDCDCAlignmentValidation_output')
    args = parser.parse_args()

    run_validation(args.calibration_results_dir[0], output_dir=args.output_dir)

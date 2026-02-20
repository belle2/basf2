#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from pathlib import Path

import matplotlib
import matplotlib.pyplot as plt

from prompt import ValidationSettings
import alignment.validation as validation

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

    validation.main("cosmics", pattern_cosmic, output_dir=output_dir)
    validation.main("dimuon", pattern_mumu, output_dir=output_dir)


if __name__ == '__main__':

    import argparse
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

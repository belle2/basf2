# -*- coding: utf-8 -*-

"""
Airflow script for TOP post-tracking calibration:
   BS13d carrier shifts, module T0 and common T0

Author: Marko Staric
"""

from prompt import CalibrationSettings
from caf.framework import Calibration
from top_calibration import BS13d_calibration_cdst
from top_calibration import moduleT0_calibration_DeltaT, moduleT0_calibration_LL
from top_calibration import commonT0_calibration_BF, commonT0_calibration_LL


#: Required variable - tells the automated system some details of this script
settings = CalibrationSettings(name="TOP post-tracking calibration",
                               expert_username="staric",
                               description=__doc__,
                               input_data_formats=["cdst"],
                               input_data_names=["hlt_bhabha"],
                               depends_on=[])


# Required function
def get_calibrations(input_data, **kwargs):
    '''
    Returns a list of calibration objects.
    :input_data (dict): Contains every file name from the 'input_data_names' as a key.
    :**kwargs: Configuration options to be sent in.
    '''

    file_to_iov = input_data["hlt_bhabha"]
    sample = 'bhabha'
    inputFiles = list(file_to_iov.keys())

    cal = [BS13d_calibration_cdst(inputFiles),
           moduleT0_calibration_DeltaT(inputFiles),
           moduleT0_calibration_LL(inputFiles, sample),
           commonT0_calibration_BF(inputFiles)]

    cal[1].save_payloads = False

    cal[1].depends_on(cal[0])
    cal[2].depends_on(cal[1])
    cal[3].depends_on(cal[2])

    return cal

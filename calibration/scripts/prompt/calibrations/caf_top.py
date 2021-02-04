# -*- coding: utf-8 -*-

"""
Airflow script for TOP post-tracking calibration:
   BS13d carrier shifts, module T0 and common T0

Author: Marko Staric
"""

from prompt import CalibrationSettings
from caf.utils import vector_from_runs, IoV, ExpRun
from caf.strategies import SingleIOV, SequentialBoundaries
from top_calibration import BS13d_calibration_cdst
from top_calibration import moduleT0_calibration_DeltaT, moduleT0_calibration_LL
from top_calibration import commonT0_calibration_BF


#: Required variable - tells the automated system some details of this script
settings = CalibrationSettings(name="TOP post-tracking calibration",
                               expert_username="skohani",
                               description=__doc__,
                               input_data_formats=["cdst"],
                               input_data_names=["hlt_bhabha"],
                               depends_on=[],
                               expert_config={"payload_boundaries": None})


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
    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    cal = [  # BS13d_calibration_cdst(inputFiles),  # this is run-dep
        moduleT0_calibration_DeltaT(inputFiles),  # this cal cannot span across experiments
        #      moduleT0_calibration_LL(inputFiles, sample),  # this cal cannot span across experiments
        # commonT0_calibration_BF(inputFiles)
    ]

    for c in cal:
        if c.strategies == SingleIOV:

            # if payload boundaries are set, turn all the SingleIOV strategies into SequentialBoundaries
            # and set the iovs that were passed via expert_config
            if expert_config["payload_boundaries"] is not None:
                c.strategies = SequentialBoundaries
                payload_boundaries = [ExpRun(output_iov.exp_low, output_iov.run_low)]
                payload_boundaries.extend([ExpRun(*boundary) for boundary in expert_config["payload_boundaries"]])
                basf2.B2INFO(f"Expert set payload boundaries are: {expert_config['payload_boundaries']}")

                for col in c.collectors:
                    col.param('granularity', 'run')

                for alg in c.algorithms:
                    alg.params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}

            else:
                for alg in c.algorithms:
                    alg.params = {"apply_iov": output_iov}
        else:
            for alg in c.algorithms:
                alg.params = {"iov_coverage": output_iov}

    # cal[1].save_payloads = False

    # cal[1].depends_on(cal[0])
    # cal[2].depends_on(cal[1])
    # cal[3].depends_on(cal[2])

    return cal

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Find expected energies for ee5x5 single crystal energy calibration.
Requires Bhabha mc in mdst format, plus ECLCalDigits"""

# ..This calibration is run manually on specially requested MC samples.
#
#   nohup b2caf-prompt-run LSF caf_ecl_ee_expected.json input_eeMC.json --permissive &
#
#   The location of the steering file caf_ecl_ee_expected.py must be specified in
#   the caf control json, caf_ecl_ee_expected.json, e.g.
#       "caf_script":
#               "/home/belle2/czhearty/users2022/BII-9711-remove-eclDigits/
#                calibration/scripts/notPrompt/calibrations/caf_ecl_ee_expected.py",
#   Environment variable BELLE2_RELEASE_DIR is useful for this purpose.


from prompt import CalibrationSettings

# --------------------------------------------------------------
# ..Tell the automated script some required details
settings = CalibrationSettings(
    name="ecl_ee_expected",
    expert_username="hearty",
    description=__doc__,
    input_data_formats=["mdst"],
    input_data_names=["bhabha_mc"],
    depends_on=[],
    expert_config={"ee5x5_min_entries": 100})

# --------------------------------------------------------------
# ..The calibration functions


def get_calibrations(input_data, **kwargs):
    import basf2
    from ROOT import Belle2
    from caf.utils import IoV
    from caf.framework import Calibration

    # --------------------------------------------------------------
    # ..Bhabha

    # ..Input data
    file_to_iov_bhabha = input_data["bhabha_mc"]
    input_files_bhabha = list(file_to_iov_bhabha.keys())

    # ..Algorithm
    algo_ee5x5 = Belle2.ECL.eclee5x5Algorithm()
    expert_config = kwargs.get("expert_config")
    ee5x5minEntries = expert_config["ee5x5_min_entries"]
    algo_ee5x5.setMinEntries(ee5x5minEntries)
    algo_ee5x5.setPayloadName("ECLExpee5x5E")
    algo_ee5x5.setStoreConst(True)

    # ..The calibration
    eclee5x5_collector = basf2.register_module("eclee5x5Collector")
    eclee5x5_collector.param("thetaLabMinDeg", 17.)
    eclee5x5_collector.param("thetaLabMaxDeg", 150.)
    eclee5x5_collector.param("minE0", 0.45)
    eclee5x5_collector.param("minE1", 0.40)
    eclee5x5_collector.param("maxdThetaSum", 2.)
    eclee5x5_collector.param("dPhiScale", 1.)
    eclee5x5_collector.param("maxTime", 10.)
    eclee5x5_collector.param("useCalDigits", True)
    eclee5x5_collector.param("requireL1", False)
    eclee5x5_collector.param("granularity", "all")
    cal_ecl_ee5x5 = Calibration("ecl_ee_expected",
                                collector=eclee5x5_collector,
                                algorithms=[algo_ee5x5],
                                input_files=input_files_bhabha
                                )
    cal_ecl_ee5x5.backend_args = {"request_memory": "4 GB"}

    # ..pre_collector_path is empty; this is mdst format data
    ee5x5_pre_path = basf2.create_path()
    cal_ecl_ee5x5.pre_collector_path = ee5x5_pre_path

    # --------------------------------------------------------------
    # ..Force the output iovs to be open
    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)
    for algorithm in cal_ecl_ee5x5.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    # --------------------------------------------------------------
    # ..Return the calibrations
    return [cal_ecl_ee5x5]

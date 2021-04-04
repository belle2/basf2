import os
import json
import subprocess
import validation_gt as vgt
import b2test_utils


with b2test_utils.clean_working_directory():

    # Prepare the configuration dictionary for CAF and dump it into a JSON file.
    config = {
        "caf_script": "caf_klm_channel_status.py",
        "database_chain": [globaltag for globaltag in reversed(vgt.get_validation_globaltags())],
        "requested_iov": [0, 0, -1, -1]
    }
    with open("config.json", "w") as config_json:
        json.dump(config, config_json)

    # Prepare the input files for the calibration and dump them into a JSON file.
    input_files = {
        "raw_physics": [
            [f"{os.environ['BELLE2_VALIDATION_DATA_DIR']}/calibration/cdst_e0008_r3121_hadronSkim/", [8, 3121]],
            [f"{os.environ['BELLE2_VALIDATION_DATA_DIR']}/calibration/cdst_e0010_r5095_hadronSkim/", [10, 5095]]
        ],
        "raw_cosmic": [
            [f"{os.environ['BELLE2_VALIDATION_DATA_DIR']}/calibration/cdst_e0008_r3121_cosmicSkim/", [8, 3121]],
            [f"{os.environ['BELLE2_VALIDATION_DATA_DIR']}/calibration/cdst_e0010_r5095_hadronSkim/", [10, 5095]]
        ],
        "raw_beam": []
    }
    with open("input_files.json", "w") as input_files_json:
        json.dump(input_files, input_files_json)

    # Now simply run the calibration locally (on our CI/CD servers we can not test different backends).
    subprocess.check_call(['b2caf-prompt-run', 'Local', 'config.json', 'input_files.json', '--heartbeat', '10'])

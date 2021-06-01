import json
import subprocess

import basf2
import b2test_utils
import validation_gt as vgt
import validationtools


calibration_path = b2test_utils.require_file('calibration', 'validation')

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
            [f"{calibration_path}/cdst_e0008_r3121_hadronSkim/", [8, 3121]],
            [f"{calibration_path}/cdst_e0010_r5095_hadronSkim/", [10, 5095]]
        ],
        "raw_cosmic": [
            [f"{calibration_path}/cdst_e0008_r3121_cosmicSkim/", [8, 3121]],
            [f"{calibration_path}/cdst_e0010_r5095_cosmicSkim/", [10, 5095]]
        ],
        "raw_beam": []
    }
    with open("input_files.json", "w") as input_files_json:
        json.dump(input_files, input_files_json)

    # Now simply run the calibration locally (on our CI/CD servers we can not test different backends).
    try:
        # validationtools.update_env()
        subprocess.check_call(['b2caf-prompt-run', 'Local', 'config.json', 'input_files.json', '--heartbeat', '20'])
    except subprocess.CalledProcessError as e:
        # In case of failure, prints the submit.sh...
        sub_name = basf2.find_file('calibration_results/KLMChannelStatus/0/collector_output/raw/0/submit.sh', '', True)
        if sub_name:
            with open(sub_name) as sub_file:
                basf2.B2ERROR('Calibration failed, here is the submit.sh of the first collector job.')
                print(sub_file.read())
        # ... and the stdout...
        out_name = basf2.find_file('calibration_results/KLMChannelStatus/0/collector_output/raw/0/stdout', '', True)
        if out_name:
            with open(out_name) as out_file:
                basf2.B2ERROR('Calibration failed, here is the stdout of the first collector job.')
                print(out_file.read())
        # ... and the stderr.
        err_name = basf2.find_file('calibration_results/KLMChannelStatus/0/collector_output/raw/0/stderr', '', True)
        if err_name:
            with open(err_name) as err_file:
                basf2.B2ERROR('Calibration failed, here is the stderr of the first collector job.')
                print(err_file.read())
        basf2.B2FATAL(
            f'The test failed because an exception was raised ({e}). Please re-run the build if this failure happened on bamboo.')

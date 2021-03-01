import os
import json
import subprocess
import validation_gt as vgt
import b2test_utils as b2u


with b2u.clean_working_directory():
    globaltags = []
    for globaltag in reversed(vgt.get_validation_globaltags()):
        globaltags.append(globaltag)
    config = {
        "caf_script": "caf_klm_channel_status.py",
        "database_chain": globaltags,
        "requested_iov": [0, 0, -1, -1]
    }

    with open("config.json", "w") as config_json:
        json.dump(config, config_json)

    input_files = {
        "raw_physics": [
            # [f"{os.environ['BELLE2_VALIDATION_DATA_DIR']}/calibration/cdst_e0008_r3121_hadronSkim/", [8, 3121]]
            ["/home/belle2/giacomo/paolo/calibration/cdst_e0008_r3121_hadronSkim/", [8, 3121]],
            ["/home/belle2/giacomo/paolo/calibration/cdst_e0010_r5095_hadronSkim/", [10, 5095]]
        ],
        "raw_cosmic": [
            ["/home/belle2/giacomo/paolo/calibration/cdst_e0008_r3121_cosmicSkim/", [8, 3121]],
            ["/home/belle2/giacomo/paolo/calibration/cdst_e0010_r5095_hadronSkim/", [10, 5095]]
        ],
        "raw_beam": []
    }

    with open("input_files.json", "w") as input_files_json:
        json.dump(input_files, input_files_json)

    subprocess.check_call(['b2caf-prompt-run', 'Local', 'config.json', 'input_files.json', '--heartbeat', '10'])

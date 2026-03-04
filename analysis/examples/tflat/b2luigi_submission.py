import os
import json
import numpy as np
import b2luigi
from sampler import main as sampler_main

# ------------------------------------------------------------------
# Read variables from config file
# ------------------------------------------------------------------
OUTPUT_PATH = os.environ.get("OUTPUT_PATH")
INPUT_DIRECTORY = os.environ.get("INPUT_DIRECTORY")
BASF2_VERSION = os.environ.get("BASF2_VERSION")
BELLE = os.environ.get("BELLE")
UNIQUEIDENTIFIER = os.environ.get("UNIQUEIDENTIFIER")
CHUNKSIZE = int(os.environ.get("CHUNKSIZE"))
WORKERS = int(os.environ.get("WORKERS"))


if not OUTPUT_PATH:
    raise RuntimeError(
        "OUTPUT_PATH not defined! Make sure config_submission.sh exports OUTPUT_PATH."
    )

if not INPUT_DIRECTORY:
    raise RuntimeError(
        "INPUT_DIRECTORY not defined! Make sure config_submission.sh exports INPUT_DIRECTORY."
    )

if not BASF2_VERSION:
    raise RuntimeError(
        "BASF2_VERSION not defined! Make sure config_submission.sh exports BASF2_VERSION."
    )

if not BELLE:
    raise RuntimeError(
        "BELLE not defined! Make sure config_submission.sh exports BELLE."
    )

if not UNIQUEIDENTIFIER:
    raise RuntimeError(
        "UNIQUEIDENTIFIER not defined! Make sure config_submission.sh exports UNIQUEIDENTIFIER."
    )


if not CHUNKSIZE:
    raise RuntimeError(
        "CHUNKSIZE not defined! Make sure config_submission.sh exports CHUNKSIZE."
    )

if not WORKERS:
    raise RuntimeError(
        "WORKERS not defined! Make sure config_submission.sh exports WORKERS."
    )


# -------------------------------------------------------------------------
# Utilities
# -------------------------------------------------------------------------

def get_all_file_paths(directory):
    if not os.path.isdir(directory):
        raise ValueError(f"Input directory does not exist: {directory}")

    file_paths = []
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(".mdst"):
                file_paths.append(os.path.join(root, file))

    if not file_paths:
        raise RuntimeError("No .mdst files found in input directory.")

    return sorted(file_paths)


def save_to_json(file_paths, output_file):
    with open(output_file, "w") as f:
        json.dump(file_paths, f, indent=2)


def get_chunked_array(filename):
    """Load a JSON array and split it into chunks."""
    with open(filename) as f:
        tab = json.load(f)

    n_chunks = max(int(len(tab) / CHUNKSIZE), 1)
    return np.array_split(tab, n_chunks)


# ------------------------------------------------------------------
# Define b2luigi tasks
# ------------------------------------------------------------------

class RecoTask(b2luigi.Task):
    """Task to run the sampler on one chunk."""

    queue = "l"

    key = b2luigi.Parameter()
    num = b2luigi.IntParameter()

    def output(self):
        yield self.add_to_output("output_file.root")

    def run(self):
        inputfile = DICO_DATA[self.key][self.num].tolist()

        print(str(inputfile))

        output_path = os.path.join(
            OUTPUT_PATH,
            f"key={self.key}",
            f"num={self.num}"
        )
        os.makedirs(output_path, exist_ok=True)

        sampler_main(
            uniqueIdentifier=UNIQUEIDENTIFIER,
            inputfile=inputfile,
            working_dir=output_path,
            is_belle=BELLE
        )


class Wrapper(b2luigi.WrapperTask):
    """Wrapper to run all RecoTasks."""

    def requires(self):
        for key in DICO_DATA.keys():
            for num in range(len(DICO_DATA[key])):
                yield RecoTask(key=key, num=num)

# ------------------------------------------------------------------
# Main
# ------------------------------------------------------------------


if __name__ == "__main__":

    # Make sure output directory exists
    os.makedirs(OUTPUT_PATH, exist_ok=True)

    # b2luigi configuration
    b2luigi.set_setting("batch_system", "lsf")
    b2luigi.set_setting("result_dir", OUTPUT_PATH)
    b2luigi.set_setting("log_dir", OUTPUT_PATH)
    b2luigi.set_setting("executable", ["python3"])
    b2luigi.set_setting("env_script", "config_submission.sh")

    # ---------------------------------------------------------------------
    # Prepare input files
    # ---------------------------------------------------------------------

    file_paths = get_all_file_paths(INPUT_DIRECTORY)

    json_path = "./Input_files.json"
    save_to_json(file_paths, json_path)

    DICO_DATA = {}
    DICO_DATA["TFLaT"] = get_chunked_array(json_path)

    print(f"Found {len(file_paths)} .mdst files")
    print(f"Submitting {len(DICO_DATA['TFLaT'])} jobs")
    print(f"Using basf2 version: {BASF2_VERSION}")

    if (BELLE):
        print("Processing Belle mdst Files using B2BII")
    else:
        print("Processing Belle II mdst Files")

    # Launch the workflow
    b2luigi.process(
        Wrapper(),
        batch=True,
        workers=WORKERS
    )

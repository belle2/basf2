import os
import json
import sys
import argparse
import b2luigi
from sampler import main as sampler_main


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


def chunk_list(lst, chunk_size):
    if chunk_size <= 0:
        raise ValueError("chunksize must be > 0")

    return [lst[i:i + chunk_size] for i in range(0, len(lst), chunk_size)]


# -------------------------------------------------------------------------
# Luigi Tasks
# -------------------------------------------------------------------------

class RecoTask(b2luigi.Task):

    queue = "l"

    key = b2luigi.Parameter()
    num = b2luigi.IntParameter()

    def output(self):
        return self.add_to_output(f"chunk_{self.num}/done.txt")

    def run(self):
        workdir = os.path.join(
            output_directory,
            f"key={self.key}",
            f"num={self.num}"
        )
        os.makedirs(workdir, exist_ok=True)

        sampler_main(
            uniqueIdentifier,
            DICO_DATA[self.key][self.num],
            working_dir=workdir,
            is_belle=is_belle
        )

        with self.output().open("w") as f:
            f.write("done\n")


class Wrapper(b2luigi.WrapperTask):

    def requires(self):
        for key in DICO_DATA:
            for num in range(len(DICO_DATA[key])):
                yield RecoTask(key=key, num=num)


# -------------------------------------------------------------------------
# Main
# -------------------------------------------------------------------------

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Submit Belle sampling jobs")

    parser.add_argument("--Input_Directory", required=True)
    parser.add_argument("--Output_Directory", required=True)
    parser.add_argument("--basf2_version", required=True)
    parser.add_argument("--Belle", type=bool, default=True)
    parser.add_argument("--uniqueIdentifier", default="TFlaT_MC16rd_light_2601_hyperion")
    parser.add_argument("--chunksize", type=int, default=1)
    parser.add_argument("--workers", type=int, default=200)

    args = parser.parse_args()
    sys.argv = sys.argv[:1]

    input_directory = os.path.abspath(args.Input_Directory)
    output_directory = os.path.abspath(args.Output_Directory)
    basf2_version = args.basf2_version
    uniqueIdentifier = args.uniqueIdentifier
    is_belle = args.Belle
    CHUNKSIZE = args.chunksize

    os.makedirs(output_directory, exist_ok=True)

    # ---------------------------------------------------------------------
    # Inline Bash Wrapper for basf2 activation
    # ---------------------------------------------------------------------

    bash_command = (
        "set -e; "
        "source /cvmfs/belle.cern.ch/tools/b2setup; "
        f"b2setup {basf2_version}; "
        "exec python3 \"$@\""
    )

    b2luigi.set_setting(
        "executable",
        ["bash", "-c", bash_command]
    )

    b2luigi.set_setting("batch_system", "lsf")
    b2luigi.set_setting("result_dir", output_directory)
    b2luigi.set_setting("log_dir", output_directory)

    # ---------------------------------------------------------------------
    # Prepare input files
    # ---------------------------------------------------------------------

    file_paths = get_all_file_paths(input_directory)

    json_path = os.path.join(input_directory, "Input_files.json")
    save_to_json(file_paths, json_path)

    DICO_DATA = {}
    DICO_DATA["input_files"] = chunk_list(file_paths, CHUNKSIZE)

    print(f"Found {len(file_paths)} .mdst files")
    print(f"Submitting {len(DICO_DATA['input_files'])} jobs")
    print(f"Using basf2 version: {basf2_version}")

    if (is_belle):
        print("Processing Belle mdst Files using B2BII")
    else:
        print("Processing Belle II mdst Files")

    # ---------------------------------------------------------------------
    # Launch
    # ---------------------------------------------------------------------

    b2luigi.process(
        Wrapper(),
        batch=True,
        workers=args.workers
    )

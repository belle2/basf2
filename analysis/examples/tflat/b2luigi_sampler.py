import glob
import argparse
import os
import b2luigi
from sampler import main as sampler_main


class SamplerTask(b2luigi.Task):
    """Sample TFlaT variables from input file."""

    queue = "l"

    num = b2luigi.IntParameter(significant=True)
    input_file = b2luigi.Parameter(significant=False)
    output_dir = b2luigi.Parameter(significant=False)
    uniqueIdentifier = b2luigi.Parameter(significant=False)
    is_belle = b2luigi.BoolParameter(significant=False)

    def output(self):
        yield self.add_to_output("output_file.root")

    def run(self):

        output_path = os.path.join(
            self.output_dir,
            f"num={self.num}"
        )
        os.makedirs(output_path, exist_ok=True)

        sampler_main(
            uniqueIdentifier=self.uniqueIdentifier,
            inputfile=self.input_file,
            working_dir=output_path,
            sampler_id=self.num,
            is_belle=self.is_belle
        )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Sample TFlaT training data')
    parser.add_argument(
        '--output_dir',
        dest='output_dir',
        type=str,
        help='Path to directory where sampled root files will be saved to'
    )
    parser.add_argument(
        '--input_dir',
        dest='input_dir',
        type=str,
        help='Path to directory where .mdst (.root) files are stored'
    )
    parser.add_argument(
        '--uniqueIdentifier',
        dest='uniqueIdentifier',
        type=str,
        default="TFlaT_Belle_light_2601_hyperion",
        help='Name of both the config .yaml to be used and the produced weightfile'
    )
    parser.add_argument(
        '--is_belle',
        dest='is_belle',
        type=bool,
        default=False,
        help='If True, sample .mdst files with Belle MC'
    )

    args, _ = parser.parse_known_args()
    output_dir = args.output_dir
    input_dir = args.input_dir
    uniqueIdentifier = args.uniqueIdentifier
    is_belle = args.is_belle

    # Make a list of input files
    if is_belle:
        files = sorted(glob.glob(os.path.join(input_dir, "*.mdst")))
    else:
        files = sorted(glob.glob(os.path.join(input_dir, "*.root")))

    print(f"Found {len(files)} files for sampling")

    # Make sure output directory exists
    os.makedirs(output_dir, exist_ok=True)

    # b2luigi configuration
    b2luigi.set_setting("batch_system", "lsf")
    b2luigi.set_setting("result_dir", output_dir)
    b2luigi.set_setting("log_dir", output_dir)
    b2luigi.set_setting("executable", ["python3"])
    b2luigi.set_setting("env_script", "setup_basf2.sh")
    b2luigi.set_setting("task_cmd_additional_args",
                        ["--output_dir",
                         output_dir,
                         "--input_dir",
                         input_dir,
                         "--uniqueIdentifier",
                         uniqueIdentifier,
                         "--is_belle",
                         str(is_belle)])

    # Launch the workflow
    b2luigi.process([SamplerTask(num=i, input_file=file, output_dir=output_dir, uniqueIdentifier=uniqueIdentifier,
                                 is_belle=is_belle) for i, file in enumerate(files)],
                    batch=True, workers=200, ignore_additional_command_line_args=True)

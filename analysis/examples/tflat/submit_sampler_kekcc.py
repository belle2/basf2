import glob
import argparse
import os

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
    parser.add_argument(
        '--channel',
        metavar='channel',
        dest='channel',
        type=str,
        default='nunu',
        help='Sampler channel: nunu or jpsiks'
    )

    args, _ = parser.parse_known_args()
    output_dir = args.output_dir
    input_dir = args.input_dir
    uniqueIdentifier = args.uniqueIdentifier
    is_belle = args.is_belle
    channel = args.channel

    # Make a list of input files
    if is_belle:
        files = sorted(glob.glob(os.path.join(input_dir, "*.mdst")))
    else:
        files = sorted(glob.glob(os.path.join(input_dir, "*.root")))

    print(f"Found {len(files)} files for sampling")

    log_dir = os.path.join(output_dir, 'logs')
    # Make sure output directory exists
    os.makedirs(log_dir, exist_ok=True)

    njobs = 0
    for sampler_id, file in enumerate(files):

        # If the output file exists and the log file indicates success, skip the job.
        output_file_name = os.path.join(output_dir, uniqueIdentifier + f'_training_data{sampler_id}.root')
        log_file_name = f'{log_dir}/{uniqueIdentifier}_{sampler_id}.log'
        if os.path.isfile(output_file_name) and os.path.isfile(log_file_name):
            with open(log_file_name, 'r') as log_file:
                content = log_file.read()
                if '\nSuccessfully completed.\n' in content:
                    continue
        # remove old log file before submission to avoid later interference with checking for success
        if os.path.isfile(log_file_name):
            os.remove(log_file_name)

        # remove old .root file before submission to avoid interference with flavorTagger
        if os.path.isfile(output_file_name):
            os.remove(output_file_name)

        # Submit job to create output file
        os.system(
            f'bsub -q s -o {log_file_name} python3 sampler.py --uniqueIdentifier {uniqueIdentifier}'
            f' --inputfile {file} --working_dir {output_dir} --BELLE {str(is_belle)} --sampler_id {sampler_id}'
            f' --channel {channel}')

        njobs += 1

    if njobs == len(files):
        print(f"Submitted {njobs} jobs to queue to create {njobs} output files")
    else:
        print(f"Submitted {njobs} jobs to queue to create {njobs} missing output files."
              f" {len(files)-njobs} existing files skipped.")

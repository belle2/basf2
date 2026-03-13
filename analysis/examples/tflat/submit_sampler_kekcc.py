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

    njobs = 0
    for sampler_id, file in enumerate(files):

        # Do nothing if output file already exists
        output_file_name = os.path.join(output_dir, uniqueIdentifier + f'_training_data{sampler_id}.root')
        if os.path.isfile(output_file_name):
            continue

        # Submit job to create output file
        os.system(
            f'bsub -q s python3 sampler.py --uniqueIdentifier {uniqueIdentifier} --inputfile {file}'
            f' --working_dir {output_dir} --BELLE {str(is_belle)} --sampler_id {sampler_id}')

        njobs += 1

    print(f"Submitted {njobs} jobs to queue to create {njobs} missing output files")

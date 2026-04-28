import os
import glob
import shutil
import argparse

import fei


def copy_files(input_path, output_path, training_data=False):
    """
    Copy specific files from the input training collection to a new collection used for FEI rebasing

    Args:
        input_path (str): Path to the source directory containing the files to copy.
        output_path (str): Path to the destination directory where selected files will be copied.
        training_data (bool): If True, copy training data files.
    """
    if os.path.exists(output_path):
        shutil.rmtree(output_path)
    os.makedirs(output_path)
    os.makedirs(os.path.join(output_path, 'collection'))

    # copy localdb
    shutil.copytree(
        os.path.join(input_path, 'collection', 'localdb'),
        os.path.join(output_path, 'collection', 'localdb'))

    # copy Summary.pickle.backup_previous
    shutil.copy(
        os.path.join(input_path, 'collection', 'Summary.pickle'),
        os.path.join(output_path, 'collection', 'Summary.pickle.backup_previous'))

    # copy non fake .xml and logs
    xml_files = glob.glob(os.path.join(input_path, 'collection', '*.xml'))
    for xml_file in xml_files:
        if not fei.core.Teacher.check_if_weightfile_is_fake(xml_file):
            shutil.copy(xml_file, os.path.join(output_path, 'collection'))
            shutil.copy(xml_file.replace('.xml', '.log'), os.path.join(output_path, 'collection'))

    # copy Monitor_TrainingData.root and Monitor_ModuleStatistics.root
    shutil.copy(
        os.path.join(input_path, 'collection', 'Monitor_TrainingData.root'),
        os.path.join(output_path, 'collection', 'Monitor_TrainingData.root'))
    shutil.copy(
        os.path.join(input_path, 'collection', 'Monitor_ModuleStatistics.root'),
        os.path.join(output_path, 'collection', 'Monitor_ModuleStatistics.root'))

    if training_data:
        shutil.copy(
            os.path.join(input_path, 'collection', 'training_input.root'),
            os.path.join(output_path, 'collection', 'training_input.root'))
        shutil.copy(
            os.path.join(input_path, 'collection', 'validation_input.root'),
            os.path.join(output_path, 'collection', 'validation_input.root'))


# ================================================================
if __name__ == "__main__":
    # python3 copy_for_rebasing.py -i B_reduced_train -o B_reduced_rebased -t
    parser = argparse.ArgumentParser(description='Copy files for re-basing FEI examples.')
    parser.add_argument('-i', '--input_path', type=str, required=True,
                        help='Path to the source directory containing the files to copy.')
    parser.add_argument('-o', '--output_path', type=str, required=True,
                        help='Path to the destination directory where selected files will be copied.')
    parser.add_argument('-t', '--training_data', action='store_true', help='Copy training data files.')

    args = parser.parse_args()
    copy_files(args.input_path, args.output_path, args.training_data)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import os
import re
import argparse
import sys
import glob
import subprocess
import yaml
import shutil


def get_argument_parser():
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    actionType = parser.add_mutually_exclusive_group(required=True)
    actionType.add_argument(
         "--lpns", action="store_true", help="Flag to request LPNs. Must be set up in gbasf2."
    )
    actionType.add_argument(
        "--yamls",
        action="store_true",
        help="Flag to request yamls. Must be set up in basf2. LPNs must already exist (use --lpns to generate).")
    actionType.add_argument(
        "--jsons",
        action="store_true",
        help="Flag to request jsons. Must be set up in basf2. yamls must already exist (use --yamls to generate).")
    actionType.add_argument(
        "--stats_submit", action="store_true", help="Flag to request stats. Must be set up in basf2."
    )
    actionType.add_argument(
        "--stats_print", action="store_true", help="Flag to request stats printing. Must be set up in basf2."
    )
    actionType.add_argument(
        "--register",
        action="store_true",
        help="Flag to register skims using gbasf2. Must be set up in gbasf2. jsons and skim script must already exist.")
    actionType.add_argument(
         "--approve", action="store_true", help="Flag to approve all skims in ToApprove state."
    )
    actionType.add_argument(
         "--upload", action="store_true", help="Flag to upload skim scripts for all skims in Initialized or Registered state."
    )

    campaignType = parser.add_mutually_exclusive_group(
        required=(not {'--stats_submit', '--stats_print', '--upload', '--approve'} & set(sys.argv)))
    campaignType.add_argument(
         "--mcri", action="store_true", help="Flag to indicate the LPNs are for run-independent MC."
    )
    campaignType.add_argument(
        "--mcrd", action="store_true", help="Flag to indicate the LPNs are for run-dependent MC."
    )
    campaignType.add_argument(
        "--data", action="store_true", help="Flag to indicate the LPNs are for data."
    )

    parser.add_argument(
        "--camp",
        required=(not {'--stats_submit', '--stats_print', '--upload', '--approve', '--register', '--yamls'} & set(sys.argv)),
        help="""Requested campaign. Use corresponding data campaign for run-dependent MC.
        Examples: MC15ri_b, bucket28, proc13exp7, or from [mc15ri, prompt, chunk1, chunk2, chunk3, chunk4, chunk5, proc13, data]"""
    )

    parser.add_argument(
        "--energy",
        choices=("4S", "4S_offres", "4S_scan", "5S_scan"),
        required=False,
        help="Beam energy level of LPNs. If not specified, finds the correct energy for data/mcrd, or will set it to 4S for mcri.",
    )

    parser.add_argument(
        "--mctype",
        required=False,
        help="""Event type of MC samples. Required for MC. Not necessary for data.
        If not specified, uses 'standard' for mcri and 'rundep' for mcrd""",
    )

    parser.add_argument(
        "--bg",
        choices=("BGx0", "BGx1", "BGx2"),
        default="BGx1",
        help="""Beam background level of MC samples. If not specified, defaults to BGx1.
        Required for MCri. Not necessary for MCrd or data.""",
    )

    parser.add_argument(
        "--genskim",
        choices=("all", "hadron", "both"),
        default="both",
        help="""General skim name of data samples.
        If not specified, defaults to both (all and hadron). Required for data. Not necessary for MC.""",
    )

    parser.add_argument(
        "-s",
        "--skims",
        required=(("--jsons" or "--stats_submit" or "--stats_print" or "--register") in sys.argv),
        nargs="+",
        metavar="CombinedSkim",
        help="List of skims to produce json request files for. Required if --jsons used."
    )

    parser.add_argument(
        "-rel",
        "--release",
        type=str,
        required=False,
        help="""Release to use for jsons and registering.
        Required if --jsons or --register selected. If not specified the one in infoYaml is used.""",
    )

    parser.add_argument(
        "-analysis_GT",
        "--analysis_GT",
        type=int,
        default=False,
        required=False,
        help="Required if one/more skims in the combined list need the analysis GT",
    )

    parser.add_argument(
        "-PID_GT",
        "--PID_GT",
        type=int,
        default=False,
        required=False,
        help="Required if one/more skims in the combined list need the PID GT",
    )

    parser.add_argument(
        "-flagged",
        "--flagged",
        action="store_true",
        help="if added use flagged skim",
    )

    parser.add_argument(
        "-dir",
        "--base_dir",
        type=str,
        default='./SkimProd/',
        help="base directory for the output",
    )

    parser.add_argument(
        "-inputYaml",
        "--inputYaml",
        type=str,
        default='flaggedSkim.yaml',
        help="input yaml file containing flagged skims definitions",
    )

    parser.add_argument(
        "-infoYaml",
        "--infoYaml",
        type=str,
        default='infoYaml.yaml',
        help="yaml containing the default release and the GTs",
    )

    parser.add_argument(
        "-collectionYaml",
        "--collectionYaml",
        type=str,
        default='collectionYaml.yaml',
        help="yaml containing the default release and the GTs",
    )

    return parser


def init_info(info):

    with open(info, 'r') as file:
        yaml_info = yaml.safe_load(file)
    yaml_info = dict(yaml_info)

    infoDict = {}
    infoDict['prodRelease'] = yaml_info['release']
    infoDict['fei_gt'] = yaml_info['GT']['FEI']
    infoDict['ana_gt'] = yaml_info['GT']['analysis']
    infoDict['pid_gt'] = yaml_info['GT']['PID']
    infoDict['campaign'] = yaml_info['campaign']

    return infoDict


def getLPNs(collection, dataset_type, lpn_dir, mctype=None):  # Only works for proc13+prompt and MC15rd (not MCri)
    with open(collection, 'r') as file:
        yaml_collection = yaml.safe_load(file)
    collectionDict = dict(yaml_collection)

    if 'MC' in dataset_type:
        col_dir = '/belle/collection/MC/'
    elif 'data' in dataset_type:
        col_dir = '/belle/collection/Data/'
    else:  # for the test and for debugging purposes
        col_dir = '/belle/collection/test/'
    for key in collectionDict.keys():
        if 'data' in dataset_type and 'MC' in key:
            continue
        if 'MC' in dataset_type and 'MC' not in key:
            continue
        for col in collectionDict[key]:
            command = f'gb2_ds_search collection --list_datasets {col_dir}{col}'
            print(f'executed: {command}')
            result = subprocess.run(command.split(), capture_output=True, text=True, )
            with open(f'{lpn_dir}{col}_LPNs.txt', 'w') as file:
                file.write(result.stdout)
            if result.returncode != 0:
                print(f"Error executing command: {result.stderr}")

    # Split up the collection .txt files based on experiment and prod number:

    # Create a dictionary to store lines grouped by experiment and prod numbers
    lines_by_experiment_and_prod = {}

    # Define regular expressions to match experiment and prod numbers
    experiment_pattern = re.compile(r'e0\d+')
    prod_pattern = re.compile(r'prod\d+')

    # Iterate through each .txt file in the directory
    for filename in os.listdir(lpn_dir):
        if filename.endswith(".txt"):
            file_path = os.path.join(lpn_dir, filename)
            with open(file_path, 'r') as file:
                for line in file:
                    # Extract experiment and prod numbers using regular expressions
                    experiment_match = experiment_pattern.search(line)
                    prod_match = prod_pattern.search(line)

                    if experiment_match and prod_match:
                        experiment_number = experiment_match.group()
                        prod_number = prod_match.group()

                        # Create a unique key for this combination of experiment and prod numbers
                        key = f"{filename.rstrip('_LPNs.txt')}_{experiment_number}_{prod_number}_LPNs"
                        # Add the line to the corresponding list in the dictionary
                        if key not in lines_by_experiment_and_prod:
                            lines_by_experiment_and_prod[key] = []
                        lines_by_experiment_and_prod[key].append(line)

    # Write lines to separate files based on experiment and prod numbers
    for key, lines in lines_by_experiment_and_prod.items():
        output_filename = os.path.join(os.path.dirname(lpn_dir.rstrip('/')), f"{key}.txt")
        with open(output_filename, 'w') as output_file:
            output_file.writelines(lines)


def getYAMLs(dataset_type, lpn_dir, yaml_dir, bg=None):  # , mcCamp=None):

    if dataset_type == 'data':
        lpn_main_dir = lpn_dir
    else:
        lpn_main_dir = lpn_dir.split('collections/')[0]

    file_list = os.listdir(lpn_main_dir)
    print('n yaml files to be created: ', len(file_list))

    for filename_lpns in file_list:
        if filename_lpns.endswith('_LPNs.txt'):
            fileName = filename_lpns.split('_LPNs.txt')[0]
            print(f'Getting yaml for {filename_lpns}')
            command = f'lpns2yaml.py {lpn_main_dir}{filename_lpns} -o {yaml_dir}{fileName}_registry.yaml --{dataset_type}'
            if bg:
                command += f' --bg {bg}'
            print(f'executed: {command}')
            result = subprocess.run(command.split(), capture_output=True, text=True)
            if result.returncode != 0:
                print(f"Error executing command: {result.stderr}")


def lines_that_contain(string, fp):
    return [line.rstrip() for line in fp if string in line]


def getJSONs(
        info,
        dataset_type,
        skim=None,
        yaml_dir=None,
        stats_dir=None,
        json_dir=None,
        mctype=None,
        release=None,
        flagged=None,
        gt=False,
        pidgt=False,
        genskim=None):

    yaml_list = os.listdir(yaml_dir)
    print('# yaml files to be checked: ', len(yaml_list))

    counter_JSONs = 0

    if dataset_type == 'mcrd':
        mctypes = mctype
        for yamlName in yaml_list:
            for mctype in mctypes:
                mctype_str = '_'+mctype+'_'
                if (yamlName.endswith('_registry.yaml') and (mctype_str in yamlName)):
                    # Get campaign info from yaml
                    campaign = ''
                    with open(yaml_dir+yamlName, 'r') as fp:
                        campaign = lines_that_contain('mcCampaign:', fp)[0]
                        campaign = campaign.split(': ')[1]
                        print('campaign:', campaign)

                    ri_rd = '--mcrd'

                    if 'FEI' in skim:
                        # for MCri we sometimes get requests to produce them in 200/fb chunks, so
                        # we need to use -N 1 argument with b2skim-prod
                        print(f'Skimming FEI. Using --analysis-globaltag {info["fei_gt"]}')
                        # TO DO: add FEI case
                        command = ''
                    else:
                        print(f'Getting jsons for combined skim {skim} on {yamlName} using release {release}. '
                              f'Unlimited runs per production.')
                        command = f'b2skim-prod {yaml_dir}{yamlName} {stats_dir}SkimStats_{skim}.json -c {campaign} ' \
                                  f' -r {release} -o {json_dir} {ri_rd} -s {skim}'
                    if gt:
                        command += f' --analysis-globaltag {info["ana_gt"]}'
                    if pidgt:
                        command += f' --pid-globaltag {info["pid_gt"]}'
                    if flagged:
                        command += ' --flagged'
                    print(command)
                    counter_JSONs += 1
                    my_proc = subprocess.Popen(
                        command.split(),
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
                    my_proc.stdin.write(b'1\n')
                    stdout, stderr = my_proc.communicate()
                    print(f'stderr = {stderr}')

    elif dataset_type == 'data':

        if (len(genskim) == 1 and genskim[0] == 'all'):
            genskim_str = '_all_'
        elif (len(genskim) == 1 and genskim[0] == 'hadron'):
            genskim_str = '_had'
        else:
            sys.exit("ERROR. Need to specify on which hlt skims run. Please indicate one using --genskim (options: all, hadron).")

        for yamlName in yaml_list:
            if (yamlName.endswith('_registry.yaml') and (genskim_str in yamlName)):
                # Get campaign info from yaml
                campaign = ''
                with open(yaml_dir+yamlName, 'r') as fp:
                    campaign = lines_that_contain('procNumber:', fp)[0]
                    campaign = campaign.split(': ')[1]
                    if 'bucket' in campaign or 'PromptExp' in campaign:
                        campaign = 'prompt'
                    print('release: ', release)
                    print('campaign:', campaign)

                if 'FEI' in skim:
                    print(f'Skimming FEI. Using --analysis-globaltag {info["fei_gt"]}')
                    command = f'b2skim-prod {yaml_dir}{yamlName} {stats_dir}SkimStats_{skim}.json -c {campaign} -r {release} ' \
                              f'-o {json_dir} --data -s {skim} --analysis-globaltag {info["fei_gt"]}'
                else:
                    command = f'b2skim-prod {yaml_dir}{yamlName} {stats_dir}SkimStats_{skim}.json -c {campaign} -r {release} ' \
                        f'-o {json_dir} --data -s {skim}'
                if gt:
                    command += f' --analysis-globaltag {info["ana_gt"]}'
                if pidgt:
                    command += f' --pid-globaltag {info["pid_gt"]}'
                if flagged:
                    command += ' --flagged'
                print(command)
                counter_JSONs += 1
                my_proc = subprocess.Popen(command.split(), stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                my_proc.stdin.write(b'1\n')
                stdout, stderr = my_proc.communicate()
                print(f'stdout = {stdout}')
                print(f'stderr = {stderr}')

    print()
    print('# JSONs created: ', counter_JSONs)


def register(dataset_type, skim=None, json_dir=None, release=None, camp=None):

    if dataset_type == 'data':
        print(f'Registering {skim}, data batch')
    else:
        print(f'Registering {skim}, MCrd batch')

    if camp:
        skimPaths = glob.glob(f'{json_dir}skim/{camp}/{release}/SkimScripts')
    else:
        skimPaths = glob.glob(f'{json_dir}skim/*/{release}/SkimScripts')

    print('# paths:', len(skimPaths))

    counter = 0
    for skimPath in skimPaths:

        jsonPath = skimPath.split('SkimScripts')[0]
        jsonPath = f'{jsonPath}*/*{skim}*.json'

        for path in glob.glob(jsonPath):
            if os.path.exists(path):
                counter = counter+1
                command = ['gb2_prod_register', path]
                print(f"Executing command in {skimPath}: {' '.join(command)}")
                result = subprocess.run(command, cwd=skimPath, capture_output=True, text=True)
                if result.returncode != 0:
                    print(f"Error executing command: {result.stderr}")
            else:
                print(f'PATH DOESN"T EXIST: {path}')

    print(f'# {counter} commands executed')


def upload_files(json_dir, release, campaignDict):
    '''
    look for all prods in "Initialized" or "Registered" state and upload the skim scripts for them
    it would be better if this could know what prods already have files uploaded
    current setup can lead to trying to re-upload a lot of scripts, although it does no harm besides waste time
    '''

    prod_cmd = 'gb2_prod_status -g skim -s all'

    try:
        result = subprocess.run(prod_cmd.split(), capture_output=True, text=True)

        if result.returncode == 0:
            output = '\n'.join(line for line in result.stdout.split('\n') if 'Initialized' in line or 'Registered' in line)
            print(output)

            prodlines = output.strip().split('\n')

            prod_list = [line.split()[0] for line in prodlines]
            camp_list = [line.split()[1] for line in prodlines]

            # upload skim files
            for prod, camp in zip(prod_list, camp_list):

                # Find the proper campaign directory according to the dictionary in the infoYaml.
                # The directory name is defined by b2skim-prod (args.json step) using the info encoded in the registry yaml
                for key, campList in campaignDict.items():
                    for campName in campList:
                        if campName in camp:
                            camp = key
                            break

                script_dir = f'{json_dir}skim/{camp}/{release}/SkimScripts'
                command = ['gb2_prod_uploadFile', '-p', prod]
                result_upload = subprocess.run(command, cwd=script_dir, capture_output=True, text=True)
                if result_upload.returncode != 0:
                    print(f"Error uploading files for prod {prod}: {result_upload.stderr}")
        else:
            print(f"Error running command: {result.stderr}")

    except (subprocess.CalledProcessError, IndexError) as e:
        print("Error while running the command. Error:", e)
        print('Probably no prods in Initialized/Registered state.')


def approve_prods():
    # look for all prods in "ToApprove" state and approve them

    prod_cmd = 'gb2_prod_status -g skim -s all'

    try:
        result = subprocess.run(prod_cmd.split(), capture_output=True, text=True)

        if result.returncode == 0:
            output = '\n'.join(line for line in result.stdout.split('\n') if 'ToApprove' in line)
            print(output)

            prodlines = output.strip().split('\n')

            prod_list = [line.split()[0] for line in prodlines]

            # approve prods
            for prod in prod_list:
                result_approve = subprocess.run(['gb2_prod_approve', '-p', prod], capture_output=True, text=True)
                if result_approve.returncode != 0:
                    print(f"Error approving prod {prod}: {result_approve.stderr}")
        else:
            print(f"Error running command: {result.stderr}")

    except (subprocess.CalledProcessError, IndexError) as e:
        print("Error while running the command. Error:", e)
        print('Probably no prods in ToApprove state.')


def cleanDirectory(lpn_dir):
    # no longer relevant with lpn search using collection
    # removes empty files in lpn list directory, in case you search for lpns that don't exist
    # e.g. selecting "--camp allprompt and --energy all" will result in a bunch of empty files

    for root, dirs, files in os.walk(lpn_dir):
        for d in [".", ".."]:
            if d in dirs:
                dirs.remove(d)
        for f in files:
            fullname = os.path.join(root, f)
            try:
                if os.path.getsize(fullname) == 0:
                    print(f'removing {fullname}')
                    os.remove(fullname)
            except BaseException:
                print(f"Can't remove {fullname}")


def getGenSkim(gen):
    # Set genskim list for data
    if gen == "all":
        genskim = ["all"]
    elif gen == "hadron":
        genskim = ["hadron"]
    elif gen == "both":
        genskim = ["all", "hadron"]

    return genskim


def getRelease(info, rel):
    if rel:
        release = rel
    else:
        release = info['prodRelease']
    print('Release set to ' + release)
    return release


def getMCtype(mctype):

    MCtypes = {
        'Main': ["mixed", "charged", "uubar", "ddbar", "ssbar", "ccbar", "taupair"],
        'LowMult': ["ee", "mumu", "gg", "eeee", "eemumu", "llXX", "hhISR"],
        'Excited': ["BstarBstar", "BstarB"],
    }

    # check if the required MCtype is allowed
    splitted_mctype = mctype.split('_')
    for s in splitted_mctype:
        if s not in MCtypes.keys():
            sys.exit("ERROR. the allowed --mctypes are: Main, LowMult, Excited and combination of them")

    out = []
    for ty, tyList in MCtypes.items():
        if ty in mctype:
            out += tyList

    return out


def getDirs(data, base_dir):

    if data:
        base_dir = base_dir + 'Data/'
    else:
        base_dir = base_dir + 'MC/'

    json_dir = base_dir
    yaml_dir = f'{base_dir}skim/yamls/'
    lpn_dir = f'{base_dir}skim/lpns/collections/'
    stats_dir = f'{base_dir}skim/stats/'

    dirList = [base_dir, json_dir, yaml_dir, lpn_dir, stats_dir]
    for folder in dirList:
        if not os.path.exists(folder):
            try:
                # Create the folder if it doesn't exist
                os.makedirs(folder)
                print(f"Folder '{folder}' created successfully.")
            except OSError as e:
                print(f"Error creating folder '{folder}': {e}")

    return json_dir, yaml_dir, lpn_dir, stats_dir


def getStats(info, skims, stats_dir, inputYaml, gt=False, pidgt=False, flagged=False):
    for skim in skims:
        print(f'Submitting combined skim {skim} for stats')
        if skim == 'FEI':
            # Include analysis globaltag for FEI
            print(f'Skimming FEI. Using --analysis-globaltag {info["fei_gt"]}')
            print(f'b2skim-stats-submit -c {stats_dir}{inputYaml.split("/")[-1]} {skim} --analysis-globaltag {info["fei_gt"]}')
            # This is the {inputYaml} in '/MC/skim/stats' folder
            command = [
                'b2skim-stats-submit',
                '-c',
                f'{stats_dir}{inputYaml.split("/")[-1]}',
                skim,
                '--analysis-globaltag',
                info["fei_gt"]]
            result = subprocess.run(command, capture_output=True, text=True)
            if result.returncode != 0:
                print(f"Error submitting stats for {skim}: {result.stderr}")
        if skim == 'SystematicsCombinedHadronic' or skim == 'SystematicsCombinedLowMulti':
            # Systematic skims get submitted as single skims (not combined)
            print(f'b2skim-stats-submit -s {skim}')
            command = ['b2skim-stats-submit', '-s', skim]
            result = subprocess.run(command, capture_output=True, text=True)
            if result.returncode != 0:
                print(f"Error submitting stats for {skim}: {result.stderr}")
        else:
            # This is the {inputYaml} in '/MC/skim/stats' folder
            command = f'b2skim-stats-submit -c {stats_dir}{inputYaml.split("/")[-1]} {skim}'
            if gt:
                print(f'Skimming {skim}. Using --analysis-globaltag {info["ana_gt"]}')
                command += f' --analysis-globaltag {info["ana_gt"]}'
            if pidgt:
                print(f'Using --pid_globaltag {info["pid_gt"]}')
                command += f' --pid-globaltag {info["pid_gt"]}'
            if flagged:
                command += ' --flagged'
            print(command)
            result = subprocess.run(command.split(), capture_output=True, text=True)
            if result.returncode != 0:
                print(f"Error executing command: {result.stderr}")


def printStats(skims, stats_dir, flagged=False):
    for skim in skims:
        if skim == 'SystematicsCombinedHadronic' or skim == 'SystematicsCombinedLowMulti':
            print(f'Printing single skim {skim} stats to json file SkimStats_{skim}.json')
            command = ['b2skim-stats-print', '-s', skim, '-J']
            result = subprocess.run(command, capture_output=True, text=True)
            if result.returncode != 0:
                print(f"Error executing command: {result.stderr}")
        else:
            print(f'Printing combined skim {skim} stats to json file SkimStats_{skim}.json')
            command = ['b2skim-stats-print', '-c', skim, '-J']
            if flagged:
                command.append('--flagged')
            result = subprocess.run(command, capture_output=True, text=True)
            if result.returncode != 0:
                print(f"Error executing command: {result.stderr}")
        print(f'Moving to {stats_dir}')
        # move to MC folder
        shutil.move('SkimStats.json', f'{stats_dir}SkimStats_{skim}.json')
        stats_dir_data = stats_dir.split('/MC/skim/stats/')[0]
        stats_dir_data = stats_dir_data+'/Data/skim/stats/'
        print(f'Copying to {stats_dir_data}')
        # make sure stats are in both MC and Data folders
        shutil.copy2(f'{stats_dir}SkimStats_{skim}.json', stats_dir_data)


def main():
    parser = get_argument_parser()
    args = parser.parse_args()

    if args.stats_submit or args.stats_print or args.jsons or args.register or args.upload:
        infoDict = init_info(args.infoYaml)

    json_dir, yaml_dir, lpn_dir, stats_dir = getDirs(args.data, args.base_dir)

    if args.lpns:
        getLPNs(collection=args.collectionYaml, dataset_type=args.camp, lpn_dir=lpn_dir)

    elif args.stats_submit:
        getStats(
            info=infoDict,
            skims=args.skims,
            stats_dir=args.base_dir,
            gt=args.analysis_GT,
            pidgt=args.PID_GT,
            flagged=args.flagged,
            inputYaml=args.inputYaml)

    elif args.stats_print:
        printStats(args.skims, stats_dir, args.flagged)

    elif args.approve:
        approve_prods()

    elif args.upload:
        release = getRelease(infoDict, args.release)
        campaignDict = infoDict['campaign']
        upload_files(json_dir, release, campaignDict)

    elif args.data:

        if args.yamls:
            getYAMLs(dataset_type='data', lpn_dir=lpn_dir, yaml_dir=yaml_dir)

        elif args.jsons or args.register:

            skims = args.skims
            release = getRelease(infoDict, args.release)
            for skim in skims:

                if args.jsons:

                    genskim = getGenSkim(args.genskim)
                    if args.genskim == "both":
                        sys.exit("ERROR. Cannot submit skims on both all and hadron. Please indicate one using --genskim.")

                    getJSONs(
                            info=infoDict,
                            dataset_type='data',
                            skim=skim,
                            yaml_dir=yaml_dir,
                            stats_dir=stats_dir,
                            json_dir=json_dir,
                            release=release,
                            flagged=args.flagged,
                            genskim=genskim,
                            gt=args.analysis_GT,
                            pidgt=args.PID_GT)

                if args.register:
                    register(
                        dataset_type='data', skim=skim, json_dir=json_dir, release=release, camp=args.camp if args.camp else None)

    elif args.mcrd:

        if args.yamls:
            getYAMLs(dataset_type='mcrd', lpn_dir=lpn_dir, yaml_dir=yaml_dir, bg=args.bg)

        elif args.jsons or args.register:

            mctypes = getMCtype(args.mctype)
            skims = args.skims
            release = getRelease(infoDict, args.release)
            for skim in skims:

                if args.jsons:
                    getJSONs(
                        info=infoDict,
                        dataset_type='mcrd',
                        skim=skim,
                        yaml_dir=yaml_dir,
                        stats_dir=stats_dir,
                        json_dir=json_dir,
                        release=release,
                        flagged=args.flagged,
                        mctype=mctypes,
                        gt=args.analysis_GT,
                        pidgt=args.PID_GT)

                if args.register:
                    register(dataset_type='mcrd', skim=skim, json_dir=json_dir, release=release)

    elif args.mcri:
        print('MC run independent is not supported anymore. Please use --mcrd.')
    if args.lpns:
        print('Checking for empty files to remove...')
        cleanDirectory(lpn_dir)


if __name__ == "__main__":
    main()

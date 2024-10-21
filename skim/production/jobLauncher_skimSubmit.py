##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import argparse
import yaml
import shutil
import subprocess
import time

"""This is a wrapper to launch skimSumbit commands easier for the entire campaign"""
"""Flagged skim implementation by default (option to pass to combined)"""

############################
# minimal example
#
# source /cvmfs/belle.kek.jp/grid/gbasf2/pro/bashrc -g belle_dataprod
# python jobLauncher_skimSubmit.py -dir ./SkimProd/ -skim skim.yaml -info info.yaml  -coll coll.yaml -s1
#
# b2setup
# python jobLauncher_skimSubmit.py -dir ./SkimProd/ -skim skim.yaml -info info.yaml  -coll coll.yaml -s2 -s3 -s4 -s5 -s6
#
# source /cvmfs/belle.kek.jp/grid/gbasf2/pro/bashrc -g belle_dataprod
# python jobLauncher_skimSubmit.py -dir ./SkimProd/ -skim skim.yaml -info info.yaml -coll coll.yaml -s7 -s8 -s9 -s10
#
############################

parser = argparse.ArgumentParser("")

# input arguments. All but release are mandatory.
parser.add_argument('-dir',
                    '--base_dir',
                    type=str,
                    default='./SkimProd/',
                    help="main output directory")
parser.add_argument('-skim',
                    '--skim',
                    type=str,
                    default='flaggedSkims.yaml',
                    help="yaml file containing skims definitions")
parser.add_argument('-info',
                    '--info',
                    type=str,
                    default='infoSkim.yaml',
                    help="python file containing HLTskim, MCtype, GT of the skims")
parser.add_argument('-coll',
                    '--collections',
                    type=str,
                    default='collections.yaml',
                    help="input collection to be used")
parser.add_argument('-rel',
                    '--release',
                    type=str,
                    default='',
                    help=" basf2 release to be used. If not provide the one in infoSkim.yaml will be used")

# configuration argument, used for debugging only
parser.add_argument('-flagged',   '--flagged',          type=int, default=True,        help="set to 0 to disable Flagged skim")
parser.add_argument('-data',   '--data',          type=int, default=True,        help="set to 0 to disabled data run")
parser.add_argument('-MC',   '--MC',          type=int, default=True,        help="set to 0 to disabled MC run")
parser.add_argument('-dry',   '--dry',          type=int, default=False,        help="set to 1 to dry run, no commands exectuted")

# steps to be run arguments (NB: you cannot mix in the same config basf2 and gbasf2 steps!)
parser.add_argument('-s1',   '--lpns',          action="store_true",        help="create LPNS (gbasf2)")
parser.add_argument('-s2',   '--yaml',          action="store_true",        help="create yaml (basf2)")
parser.add_argument('-s3',   '--stat_sub',          action="store_true",        help="create stats (basf2)")
parser.add_argument('-s4',   '--stat_print',          action="store_true",        help="print stats (basf2)")
parser.add_argument('-s5',   '--json',          action="store_true",        help="create json (basf2)")
parser.add_argument('-s6',   '--check',          action="store_true",        help="check json (basf2)")
parser.add_argument('-s7',   '--register',          action="store_true",        help="register productions (gbasf2)")
parser.add_argument('-s8',   '--upload',          action="store_true",        help="upload productions (gbasf2)")
parser.add_argument('-s9',   '--approve',          action="store_true",        help="approve productions (gbasf2)")
parser.add_argument('-s10',   '--ask',          action="store_true",        help="ask to Hideki-san to launch (copy-paste)")

args = parser.parse_args()


# retrieving the input
with open(args.skim, 'r') as file:
    yaml_skim = yaml.safe_load(file)
skims_dict = dict(yaml_skim)

with open(args.info, 'r') as file:
    yaml_info = yaml.safe_load(file)
info_dict = dict(yaml_info)

# create the output directory
if not os.path.exists(args.base_dir):
    os.makedirs(args.base_dir)
shutil.copy2(args.skim, args.base_dir)  # this is needed to be provided to skimSubmit.py in step3

if not args.lpns and not args.register and not args.upload and not args.approve and not args.ask:
    from termcolor import colored

try:
    print(colored("WARNING: check that you have all your skims not commented in the YAML", 'red'))
except BaseException:
    print("WARNING: check that you have all your skims not commented in the YAML")


if args.lpns:  # step1 - gbasf2, once per campaign, one command for data and one for MC
    print('>>>> Step 1: retrive LPNS')

    command = f'python3 skimSubmit.py --lpns  --base_dir {args.base_dir} --collectionYaml {args.collections}'

    if args.data:
        command_data = command+' --data --camp data'
        print(f'>>>> Executed command: {command_data}')
        if not args.dry:
            subprocess.run(command_data.split(), text=True)

    if args.MC:
        command_MC = command+' --mcrd --camp MC'
        print(f'>>>> Executed command: {command_MC}')
        if not args.dry:
            subprocess.run(command_MC.split(), text=True)


if args.yaml:  # step2 - basf2 once per campaign, one command for data and one for MC
    print(colored('>>>> Step 2: create yaml', 'blue', attrs=['bold']))

    command = f'python3 skimSubmit.py --yamls --base_dir {args.base_dir}'

    if args.data:
        command_data = command+' --data '
        print(colored(f'>>>> Executed command: {command_data}', 'green'))
        if not args.dry:
            subprocess.run(command_data.split(), text=True)

    if args.MC:
        command_MC = command + ' --mcrd --bkg BGx1'
        print(colored(f'>>>> Executed command: {command_MC}', 'green'))
        if not args.dry:
            subprocess.run(command_MC.split(), text=True)


if args.stat_sub:  # step3 - basf2, loop on skim, common for data and MC
    print(colored('>>>> Step 3: create stats for all the skims ', 'blue', attrs=['bold']))
    print('>>>> NB:bsub used, will take a wile to submit and to run)', 'red')

    if args.flagged:
        flaggedString = ' --flagged '
    else:
        flaggedString = ''

    for skim, skimVal in skims_dict.items():
        GTstring = ''
        for GT in info_dict[skim]['GT']:
            if GT == 'analysis':
                GTstring += ' --analysis_GT 1 '
            if GT == 'PID':
                GTstring += ' --PID_GT 1 '
        command = f'python3 skimSubmit.py --stats --skims {skim} {flaggedString} --action submit {GTstring} ' \
                  f' --base_dir {args.base_dir} --inputYaml {args.skim} --infoYaml {args.info}'
        print(colored(f'>>>> Executed command: {command}', 'green'))
        if not args.dry:
            subprocess.run(command.split(), text=True)


if args.stat_print:  # step4 - basf2, loop on skim, common for data and MC
    print(colored('>>>> Step 4: print the results of the stats', 'blue', attrs=['bold']))

    print(colored('>>>> WARNING: check if some jobs are still running. You should not have your personal jobs on queues', 'red'))
    # This checking according to `bjobs`. If you have personal jobs on l
    # queue, the script will wait them to finish (despite they are not the
    # ones of the step 3)

    jobs_notDone = True
    while jobs_notDone:
        bjobs_out = subprocess.check_output('bjobs -q l'.split(), text=True)

        if bjobs_out == '':
            jobs_notDone = False
        else:
            print(colored("Unfinished job found in queue <l>, waiting 5 minutes", 'red'))
            print(bjobs_out)
            time.sleep(300)

    if args.flagged:
        flaggedString = ' --flagged '
    else:
        flaggedString = ''

    for skim, skimVal in skims_dict.items():
        command = f'python3 skimSubmit.py --stats --skims {skim} {flaggedString} --action print ' \
                  f' --infoYaml {args.info} --base_dir {args.base_dir}'
        print(colored(f'>>>> Executed command: {command}', 'green'))
        if not args.dry:
            subprocess.run(command.split(), text=True)

if args.json:  # step5 - basf2, loop on skim, one command for data and one for MC
    print(colored('>>>> Step 5: create json', 'blue', attrs=['bold']))

    if args.flagged:
        flaggedString = ' --flagged '
    else:
        flaggedString = ''

    for skim, skimVal in skims_dict.items():
        GTstring = ''
        for GT in info_dict[skim]['GT']:
            if GT == 'analysis':
                GTstring += ' --analysis_GT 1 '
            if GT == 'PID':
                GTstring += ' --PID_GT 1 '
        HLTstring = info_dict[skim]['HLT']
        MCTypeString = info_dict[skim]['MCType']

        command = f'python3 skimSubmit.py --json --skims {skim} {flaggedString}  ' \
            f'--infoYaml {args.info} --base_dir {args.base_dir} --release {args.release}'

        if args.data:
            command_data = command+f' --data --camp data --genskim {HLTstring} {GTstring}'
            print(colored(f'>>>> Executed command: {command_data}', 'green'))
            if not args.dry:
                subprocess.run(command_data.split(), text=True)

        if args.MC:
            command_MC = command+f' --mcrd --camp mcrd --mctype {MCTypeString} {GTstring}'
            print(colored(f'>>>> Executed command: {command_MC}', 'green'))
            if not args.dry:
                subprocess.run(command_MC.split(), text=True)

if args.check:  # step6 - basf2, loop on skim, one command for data and one for MC
    print(colored('>>>> Step 6: check json', 'blue', attrs=['bold']))

    for skim, skimVal in skims_dict.items():

        command = f'python3 checkJSONs.py -s {skim} --path {args.base_dir} --verbose 1'

        if args.data:
            command_data = command+' -d Data'
            print(colored(f'>>>> Executed command: {command_data}', 'green'))
            if not args.dry:
                subprocess.run(command_data.split(), text=True)
        if args.MC:
            command_MC = command+' -d MC'
            print(colored(f'>>>> Executed command: {command_MC}', 'green'))
            if not args.dry:
                subprocess.run(command_MC.split(), text=True)

if args.register:  # step7 - gbasf2, loop on skim, one command for data and one for MC
    print('>>>> Step 7: register productions on grid')

    for skim, skimVal in skims_dict.items():

        command = f'python3 skimSubmit.py --register --skims {skim}  --infoYaml {args.info} ' \
           f'--base_dir {args.base_dir} --release {args.release}'

        if args.data:
            command_data = command+' --data'
            print(f'>>>> Executed command: {command_data}')
            if not args.dry:
                subprocess.run(command_data.split(), text=True)

        if args.MC:
            command_MC = command+' --mcrd'
            print(f'>>>> Executed command: {command_MC}')
            if not args.dry:
                subprocess.run(command_MC.split(), text=True)


if args.upload:  # step8 - gbasf2, one command for data and one for MC
    print('>>>> Step 8: upload productions')

    command = f'python3 skimSubmit.py --upload --infoYaml {args.info} --base_dir {args.base_dir} --release {args.release}'

    if args.data:
        command_data = command+' --data'
        print(f'>>>> Executed command: {command_data}')
        if not args.dry:
            subprocess.run(command_data.split(), text=True)

    if args.MC:
        command_MC = command+' --mcrd'
        print(f'>>>> Executed command: {command_MC}')
        if not args.dry:
            subprocess.run(command_MC.split(), text=True)


if args.approve:  # step9 - gbasf2, common for data and MC
    print('>>>> Step 9: approve productions')

    command_check = 'gb2_prod_status -g skim -s all --date 1w'
    status_out = subprocess.check_output(command_check.split(), text=True)
    while 'Registered' in status_out or 'Initialized' in status_out or 'ToApprove' in status_out:
        status_out = subprocess.check_output(command_check.split(), text=True)

        if 'ToApprove' in status_out:
            command = f'python3 skimSubmit.py --approve --base_dir {args.base_dir}'
            print(f'>>>> Executed command: {command}')
            if not args.dry:
                subprocess.run(command.split(), text=True)

        else:
            print("There are still productions not in 'ToApprove' status. Waiting 5 minutes")
            print(status_out)
            time.sleep(300)

if args.ask:  # step 10 - open a ticket with the produced output (output_stats.log) tagging the production system manager.
    print('>>>> Step 10: ask computing team to launch the productions')
    print('>>>> WARNING: all the Approved skim productions within last week will be listed')
    command = 'gb2_prod_expected -s Approved -g skim --date 1w'
    print(f'>>>> Executed command: {command}')
    if not args.dry:
        result = subprocess.run(command.split(), capture_output=True, text=True)
        with open('output_stats.log', 'w') as file:
            file.write(result.stdout)
        with open('output_stats_err.log', 'w') as file:
            file.write(result.stderr)

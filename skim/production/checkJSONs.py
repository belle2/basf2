#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import argparse
import glob


def lines_that_contain(string, fp):
    return [line.strip('\n\r') for line in fp if string in line]

# =========================================================


parser = argparse.ArgumentParser(description=__doc__)

parser.add_argument("-s", "--skim", nargs="+", required=False,
                    help="List of skims for which the json files should be checked.")

parser.add_argument('-d', '--dataset', choices=('Data', 'MC'), required=False,
                    help="List of skims for which the json files should be checked.")

parser.add_argument('--verbose', type=bool, default=False,
                    help='Print more information. Default %(default).')

parser.add_argument('--MAXEventCPUTime', type=float, required=False,
                    help='Maximum value for ExpectedEventCPUTime to be compared with.')

parser.add_argument('--path', type=str, default='/gpfs/group/belle2/dataprod/skim/re-skim-campaign-proc13-prompt-mc15rd/skimProd/',
                    help='absolute path of the main directory where the output file are created. Default=reskimming campaign')

args = parser.parse_args()

# =========================================================

dataset_type = '*'
if args.dataset:
    dataset_type = args.dataset
    print(f'Cheking {dataset_type}')
    print('')
else:
    print('Cheking both Data and MC')

base_path = f'{args.path}/{dataset_type}/skim/'

valueLIM = 2.5
if args.MAXEventCPUTime:
    valueLIM = args.MAXEventCPUTime
print('Looking for JSONs with ExpectedEventCPUTime > {valueLIM}')

json_path = f'{base_path}*/*/*/*.json'
if args.skim:
    skims = args.skim
    print('Checking JSONs files for the following skims:', skims)
    json_list = []
    for p in glob.glob(json_path):
        for s in skims:
            if s in p:
                json_list.append(p)

    print(len(json_list))
else:
    print('No skims specified. Checking JSONs for all skims.')
    print('If you want to check specific skims, use the -s parameter.')
    json_list = glob.glob(json_path)

if args.verbose:
    print('json_list', json_list)

print('N files to be checked: ', len(json_list))
if args.verbose:
    print('base path:', base_path)

print('')

n_problematic_files = 0
for path in json_list:
    if os.path.exists(path):

        this_file = path.split('/')[-1]
        if args.verbose:
            print(f'Checking {this_file}')
        with open(path, 'r') as fp:
            HS06 = lines_that_contain('ExpectedEventCPUTime', fp)

        for h in HS06:
            title, value = h.split(':')
            value = value.replace(',', '')
            value = float(value)
            if args.verbose:
                print(value)
            if value > valueLIM:
                print(f'Problem found! Value too high! Please check {path}')
                print(f'---> ExpectedEventCPUTime: {value}')
                print('')
                n_problematic_files += 1
        if args.verbose:
            print('')

print('')
if n_problematic_files == 0:
    print(f'No problems found. In all JSONs checked, ExpectedEventCPUTime < {valueLIM}')

else:
    print(f'WARNING: {n_problematic_files} found with ExpectedEventCPUTime > {valueLIM}.')
    print('Do not register them, take actions first.')

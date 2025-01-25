#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
This script will get all the "Done" skim productions for a given campaign, check which ones already have
metadata retrieved for, and retrieve the metadata for the remaining productions. Then, upload those
metadata to the DSS. This way, if a production is later deleted from the DSS, it won't get re-added.

Arguments:
    --campaign: any campaign name OR all, MCrd, data, prompt
    --skim: name of the combinedSkim that you want to upload to the dss

Example usage:
    python dss_upload.py --campaign all --skim P1_HLTAll_all
    python dss_upload.py --campaign MCrd --skim P1_HLTAll_all
    python dss_upload.py --campaign data --skim P1_HLTAll_all
    python dss_upload.py -- campaign proc13 --skim P1_HLTAll_all

'''

import argparse as ap
import subprocess
import os
import glob

parser = ap.ArgumentParser()
parser.add_argument('-c', '--campaign', required=True,
                    help='Campaign that you want to register')
parser.add_argument('-s', '--skim', required=True,
                    help='Combined skim that you want to register')
args = parser.parse_args()

campaign = args.campaign
skim = args.skim


MCrd = ['MC15rd_a', 'MC15rd_b']
proc13 = ['proc13']
prompt = [
    'bucket26',
    'bucket27',
    'bucket28',
    'bucket29',
    'bucket30',
    'bucket31',
    'bucket32',
    'bucket33',
    'bucket34',
    'bucket35',
    'bucket36']
data = proc13 + prompt
allSkims = MCrd + data

if campaign == 'all':
    campaign = allSkims
elif campaign == 'proc13':
    campaign = proc13
elif campaign == 'prompt':
    campaign = prompt
elif campaign == 'data':
    campaign = data
elif campaign == 'MCrd':
    campaign = MCrd
else:
    campaign = [campaign]


for camp in campaign:
    print('Working on campaign {0}'.format(camp))
    prod_list = glob.glob('{0}_done_*'.format(camp))
    prod_count = len(prod_list)

    prod_fn = '{2}_{0}_done_{1}.txt'.format(camp, prod_count, skim)
    meta_fn = '{2}_{0}_metadata_{1}.txt'.format(camp, prod_count, skim)
    failed_meta_fn = '{2}_{0}_metadata_failed_{1}.txt'.format(camp, prod_count, skim)
    reg_fn = '{2}_{0}_registered_{1}.txt'.format(camp, prod_count, skim)
    failed_reg_fn = '{2}_{0}_registered_failed_{1}.txt'.format(camp, prod_count, skim)

    # need grep -v reference in order to skim ref productions
    prod_command = "gb2_prod_status -g skim -c {0} -s Done | grep {1} | grep -v reference | " \
                   "awk '{{print $1}}' | awk 'NR>0' > {2}".format(camp, skim, prod_fn)
    print('1, ', prod_command)
    subprocess.call(prod_command, shell=True)

    if os.path.getsize(prod_fn) == 0:
        print('No Done productions for {0}\n'.format(camp))
        os.remove(prod_fn)
        continue

    if prod_count > 0:
        prev_fn = '{0}_done_{1}.txt'.format(camp, prod_count-1)
        new_fn = '{0}_new_{1}.txt'.format(camp, prod_count)
        curr = open(prod_fn).readlines()
        prev = open(prev_fn).readlines()
        for line in prev:
            if line in curr:
                curr.remove(line)
        if not curr:
            print('No new Done productions for campaign {0}\n'.format(camp))
            os.remove(prod_fn)
            continue
        with open(new_fn, 'w') as new:
            new.writelines(curr)
        new.close()

    elif prod_count == 0:
        new_fn = prod_fn

    meta_command = "gb2_prod_show_metadata -i {0} -o {1} -e {2}".format(new_fn, meta_fn, failed_meta_fn)
    print('2, ', meta_command)
    subprocess.call(meta_command, shell=True)

    if "MC" in camp or "SkimM14" in camp:
        # -bkg BGx1 needed for MC
        print('Creating dataset for MC: {0}'.format(camp))
        create_command = "gb2_ds_searcher_create dataset -i {0} -o {1} -e {2} -bkg BGx1".format(meta_fn, reg_fn, failed_reg_fn)
    else:
        print('Creating dataset for Data: {0}'.format(camp))
        create_command = "gb2_ds_searcher_create dataset -i {0} -o {1} -e {2}".format(meta_fn, reg_fn, failed_reg_fn)

    print('3, ', create_command)
    # subprocess.call(create_command, shell=True)
    print()

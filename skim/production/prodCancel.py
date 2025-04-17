##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import argparse
import subprocess

parser = argparse.ArgumentParser("")
parser.add_argument('-min',   '--min',          type=str, default='',        help="starting number of prod to cancel")
parser.add_argument('-max',   '--max',          type=str, default='',        help="ending number of prod to cancel")
parser.add_argument('-c',   '--comment',          type=str, default='',        help="comment for the cancellation")
args = parser.parse_args()

for i in range(int(args.min), int(args.max)+1):
    print(f'cancelling prod {i}')
    subprocess.run(['gb2_prod_cancel', '-p', str(i), '--comment', args.comment])

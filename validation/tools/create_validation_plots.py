#!/usr/bin/env python
# -*- encoding: utf-8 -*-

# Normal imports
import argparse
import os
import sys

from validationplots import create_plots

# Only execute the program if a basf2 release is set up!
if os.environ.get('BELLE2_RELEASE', None) is None:
    sys.exit('Error: No basf2 release set up!')

# Make sure the output of validate_basf2.py is there
if not os.path.isdir('html/results'):
    sys.exit('Error: No html/results dir found! Run validate_bash2.py first.')

# Define the accepted command line flags and read them in
parser = argparse.ArgumentParser()
parser.add_argument("-r", "--revisions", help="Takes a list of revisions ("
                                              "separated by spaces) and "
                                              "generates the plots for them",
                    type=str, nargs='*')
parser.add_argument("-f", "--force", help="Regenerates plots even if the "
                                          "requested combination exists "
                                          "already",
                    action='store_true')
args = parser.parse_args()

# Go to the html directory and call the create_plots function
save_dir = os.getcwd()
os.chdir('html')
create_plots(args.revisions, args.force)

# restore original working directory
os.chdir(save_dir)

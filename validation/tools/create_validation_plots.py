#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Normal imports
import argparse
import os
import sys

from validationplots import create_plots

# Only execute the program if a basf2 release is set up!
if os.environ.get('BELLE2_RELEASE', None) is None:
    sys.exit('Error: No basf2 release set up!')

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
save_dir = os.path.abspath(os.getcwd())
os.chdir('html')
create_plots(args.revisions, args.force, save_dir)

# restore original working directory
os.chdir(save_dir)

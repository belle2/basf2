#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import os
import json
import argparse
"""
This script adds a version number suffix to the`ProductionName` of all submission
JSONs created in a skim campaign. This allows for resubmission of a skim campaign
if a mistake has been made.
"""
parser = argparse.ArgumentParser("")
parser.add_argument("-dir", nargs="+", help="List of top-level directories that \
 contain skim campaign running files")
args = parser.parse_args()

# Iterate through all specified directories and their subdirectories
for top_directory in args.dir:
    print(f"Processing top-level directory: {top_directory}")
    for root, subdirs, files in os.walk(top_directory):
        for filename in files:
            if filename.endswith(".json"):
                filepath = os.path.join(root, filename)

                # Open and load the JSON file
                with open(filepath, 'r') as file:
                    try:
                        data = json.load(file)
                    except json.JSONDecodeError:
                        print(f"Error decoding {filename}, skipping.")
                        continue

                # Check if "ProductionName" exists and modify it
                if "ProductionName" in data:
                    # Append "_v2" to the existing ProductionName
                    data["ProductionName"] += "_v2"

                # Save the updated JSON back to the file
                with open(filepath, 'w') as file:
                    json.dump(data, file, indent=4)

                print(f"Updated {filename} in {root}")

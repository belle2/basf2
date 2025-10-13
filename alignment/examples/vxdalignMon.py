#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Generate alignment Monitoring Objects for Mirabelle
"""


import os
import re
import ROOT
from ROOT import Belle2
import numpy as np
from glob import glob
import sys

# extract alignment parameter values from the VXDAlignment payload


def get_means(payload):
    # sensor IDs for PXD and SVD
    pxd_shells = [Belle2.VxdID(1, 0, 0, 1), Belle2.VxdID(1, 0, 0, 2)]
    svd_shells = [Belle2.VxdID(3, 0, 0, 1), Belle2.VxdID(3, 0, 0, 2)]
    means = {}
    for par in range(1, 7):
        pxd_values = []
        svd_values = []
        vxd_values = []
        for shell in pxd_shells:
            v = payload.get(shell.getID(), par)
            if v != 0.:
                v = v * 1e3 if par in [4, 5, 6] else v * 1e4
                pxd_values.append(v)
                vxd_values.append(v)
        for shell in svd_shells:
            v = payload.get(shell.getID(), par)
            if v != 0.:
                v = v * 1e3 if par in [4, 5, 6] else v * 1e4
                svd_values.append(v)
                vxd_values.append(v)
        param_name = {1: "x", 2: "y", 3: "z", 4: "alpha", 5: "beta", 6: "gamma"}[par]
        means[f"pxd_{param_name}"] = np.mean(pxd_values) if pxd_values else 0.0
        means[f"svd_{param_name}"] = np.mean(svd_values) if svd_values else 0.0
        means[f"vxd_{param_name}"] = np.mean(vxd_values) if vxd_values else 0.0
    return means


def create_output_file(exp, run, means, out_dir):
    # create a root file with monitoringObject and DQMFileMetaData for a specific run
    mon = ROOT.Belle2.MonitoringObject("vxd_alignment")
    for name, value in means.items():
        mon.setVariable(name, float(value))
    meta = ROOT.Belle2.DQMFileMetaData()
    meta.setExperimentRun(exp, run)
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)
    out_file_path = os.path.join(out_dir, f"mon_exp{exp:03d}_run{run:06d}.root")
    out_file = ROOT.TFile(out_file_path, "RECREATE")
    meta.Write()
    mon.Write()
    out_file.Close()
    print(f"Created {out_file_path}")


if len(sys.argv) != 3:
    print("Usage: python merged_code.py <input_path> <output_dir>")
    sys.exit(1)

input_path = sys.argv[1]  # Directory with database.txt and payloads from DDB
output_dir = sys.argv[2]

# Read database.txt to map revision IDs to experiment/run ranges
database_file = os.path.join(input_path, "database.txt")
if not os.path.exists(database_file):
    print(f"Error: {database_file} not found.")
    sys.exit(1)

with open(database_file, "r") as db_file:
    database_lines = db_file.readlines()

# extract run range information from database.txt
database_dict = {}
for line in database_lines:
    match = re.match(r"dbstore/VXDAlignment\s+([A-Za-z0-9]{4})\s+(\d+),(\d+),(\d+),(\d+)", line.strip())
    if match:
        rev = match.group(1)
        exp1 = int(match.group(2))
        start_run = int(match.group(3))
        exp2 = int(match.group(4))
        end_run = int(match.group(5))
        database_dict[rev] = (exp1, start_run, exp2, end_run)

# Process each .root file
root_files = glob(os.path.join(input_path, "dbstore_VXDAlignment_rev_*.root"))
for root_file in root_files:
    match = re.search(r"dbstore_VXDAlignment_rev_([A-Za-z0-9]{4})\.root", root_file)
    if not match:
        print(f"Couldn't find a rev from file name: {root_file}")
        continue
    rev = match.group(1)
    if rev not in database_dict:
        print(f"No database entry found for rev: {rev}")
        continue
    # experiment and run range info from database
    exp1, start_run, exp2, end_run = database_dict[rev]
    if exp1 != exp2:
        print(f"Warning: exp1 ({exp1}) differs from exp2 ({exp2}) for rev {rev}. Using exp1.")
    exp = exp1

    f = ROOT.TFile.Open(root_file)
    if not f or not f.IsOpen():
        print(f"error: unable to open ROOT file: {root_file}")
        continue
    payload = f.Get("VXDAlignment")
    if not payload:
        print(f"error: VXDAlignment not found in {root_file}")
        f.Close()
        continue

    # Calculate mean alignment parameters
    means = get_means(payload)
    f.Close()

    # Generate output files for each run
    for run in range(start_run, end_run + 1):
        create_output_file(exp, run, means, output_dir)

print(f" Done! all ROOT files created in {output_dir}")

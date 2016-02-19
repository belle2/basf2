#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Tool to do for each git commit in a specific range:
- check out the commit (configurable if you want to run git stash and git stash pop before and afterwards)
- run scons or not (configurable)
- run a script (given on command line)
- extract the meanX, the meanY and the zeroSupressedMeanY from variables from ROOT files (also configured on the commandline)
- write all variables together with their svn number and svn log message into a pandas DataFrame and this dataframe into the
  results.pkl file as a pickle stream.

  For example if you want to examine some commits related to tracking, you can use:

  repository_validation.py 5e066d31b391bb240cb2b381e67c5488681a8ec6 05d5bf078a45f6af8022bf06b398213d0bc2409d
    --check-quantity ~/basf2/tracking/validation/results/current/TrackingValidation.root:hEfficiency
    --script "validate_basf2 -s *_trackingEfficiency_*.py"

  which will run through all commits from 5e066d31b391bb240cb2b381e67c5488681a8ec6 to 05d5bf078a45f6af8022bf06b398213d0bc2409d
  and execute "validate_basf2 -s *_trackingEfficiency_*.py" for each of them. Then it will extract the meanX, the meanY and
  the zeroSupressedMeanY of the TNtuple hEfficiency in the file
  ~/basf2/tracking/validation/results/current/tracking/TrackingValidation.root.

  The options --script and --check-quantity can be given more than once.

  Author: The Belle II Collaboration
  Contributors: Nils Braun, Thomas Hauth
"""

import ROOT
from subprocess import check_output, CalledProcessError, PIPE, check_call

import argparse

import pickle

import pandas as pd
import quantity_extract


def extract_fom_from_histogram(file_name, fom_name):
    # Read in the given root file and export the information to the results
    root_file = ROOT.TFile(file_name)
    print(file_name)
    if not root_file.IsOpen():
        print("Cannot open ROOT file {}".format(file_name))
        return None

    results = {}

    for result_key in fom_name:
        root_obj = root_file.Get(result_key)

        qe = quantity_extract.RootQuantityExtract()
        results = qe.extract(root_obj)
        # update keys with the fom's name
        results = {result_key + "_" + key: value for (key, value) in results.items()}

    root_file.Close()
    return results


def call_with_check(fail_message, show_output, *args, **kwargs):
    try:
        if show_output:
            check_call(*args, **kwargs)
        else:
            check_output(stderr=PIPE, *args, **kwargs)
        return True
    except CalledProcessError:
        print(fail_message)
        return False


def get_fom_from_file(script_name, file_names_and_fom, git_commit_hash, skip_compiling=False, use_stash=True):

    results = {}

    # Checkout git (stash or not stash)
    print("Checking out git hash {hash}.".format(hash=git_commit_hash))
    if use_stash:
        if not call_with_check("Error in checking out git hash {hash}.".format(hash=git_commit_hash), False,
                               "git stash && git checkout {hash} && git stash pop".format(hash=git_commit_hash), shell=True):
            return results

    else:
        if not call_with_check("Error in checking out git hash {hash}.".format(hash=git_commit_hash), False,
                               ["git", "checkout", git_commit_hash]):
            return results

    # Receive svn information
    svn_number = None
    svn_log = None

    # make this optional as no subversion information might be available for local git branches
    try:
        svn_number = check_output(["git", "svn", "find-rev", git_commit_hash]).decode("utf-8").split("\n")[0]
        svn_log = check_output(["git", "log", "--pretty=format:%s", "--max-count=1", git_commit_hash]).decode("utf-8")
        print("This is svn number {svn}.".format(svn=svn_number))
    except CalledProcessError:
        print("Cannot retrieve subversion information")

    results.update({"svn_number": svn_number, "svn_log": svn_log, "git_hash": git_commit_hash})

    # Compile repo
    if not skip_compiling:
        print("Compiling...")
        if not call_with_check("Error in compiling.", False, ["scons", "-j8", "-D"]):
            return results

    # Execute script
    if script_name is not None:
        script_command = " && ".join(script_name)
        print("Running the script {name}...".format(name=script_command))
        if not call_with_check("Error in executing the script.", True, script_command, shell=True):
            return results

    # Extract results
    if file_names_and_fom is not None:
        print("Checking the results...")
        for file_name in file_names_and_fom:
            results.update(extract_fom_from_histogram(file_name, file_names_and_fom[file_name]))

    return results


def go_through_git_repo(git_start_hash, git_end_hash, script_name, file_names_and_fom, skip_compiling=False, use_stash=True):
    print("Goigh through git repository from {start} to {end}.".format(start=git_start_hash, end=git_end_hash))
    try:
        git_hashs = check_output(["git", "log", str(git_start_hash + ".." + git_end_hash),
                                  "--pretty=format:%H"]).decode("utf-8").split("\n")
    except CalledProcessError:
        print("Error while receiving the git history.")
        return

    print("There are {size} commits in this range.".format(size=len(git_hashs)))

    results = []
    for git_hash in git_hashs:
        results.append(get_fom_from_file(script_name=script_name,
                                         file_names_and_fom=file_names_and_fom,
                                         git_commit_hash=git_hash,
                                         skip_compiling=skip_compiling,
                                         use_stash=use_stash))

        # Temporarily save the results into a file
        with open("tmp.pkl", "wb") as f:
            pickle.dump(results, f)

    print("Finished.")
    return pd.DataFrame(results)


# setup argument parser options
parser = argparse.ArgumentParser(description='Evaluate status of the compile and validation of a '
                                             'specific basf2 revision.')
parser.add_argument('git_start_hash',
                    help='Start git hash to check.')

parser.add_argument('git_end_hash',
                    help='End git hash to check.')

parser.add_argument('--skip-compile', action='store_true',
                    default=False,
                    help='Do not trigger a compile, useful for saving time when '
                    'setting up the quantity checks.')

parser.add_argument('--use-stash', action='store_true',
                    default=True,
                    help='Use a git stash and a git stash pop before and after the git checkout.')

parser.add_argument('--check-quantity', action="append",
                    help='Check for a quantity in validation files')

parser.add_argument('--script', action="append",
                    help='Name of validation script to run after the compile '
                         'and before the quantity check. Only this validation script and '
                         'all scripts it depends on are executed.')
args = parser.parse_args()
argsVar = vars(args)

# see if quantities must be checkd and
# make sure the expression can be properly parsed
if argsVar["check_quantity"] is None:
    argsVar["check_quantity"] = []

file_names_and_fom = {}
for c_string in argsVar["check_quantity"]:
    file_name, fom_name = c_string.split(":")
    if file_name in file_names_and_fom:
        file_names_and_fom[file_name].append(fom_name)
    else:
        file_names_and_fom[file_name] = [fom_name]

result = go_through_git_repo(argsVar["git_start_hash"], argsVar["git_end_hash"], argsVar["script"],
                             file_names_and_fom,
                             argsVar["skip_compile"], argsVar["use_stash"])

with open("result.pkl", "wb") as f:
    pickle.dump(result, f)

print(result)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
This script will get all the "Done" skim productions for a given campaign, check which ones already have
metadata retrieved for, and retrieve the metadata for the remaining productions. Then, upload those
metadata to the DSS. This way, if a production is later deleted from the DSS, it won't get re-added.

Arguments:
    --combinedSkim --skims combSkim1 combSkim2
    --singleProductions --prods xxxxx xxxxx

Example usage:
    python dss_upload.py --combinedSkim --skims P1_HLTAll_all P2_HLTAll_all
    python dss_upload.py --singleProductions -p 45073 45074

"""

import argparse as ap
import subprocess
import sys

problematic_prods = []


def get_argument_parser():

    parser = ap.ArgumentParser(
        formatter_class=ap.RawDescriptionHelpFormatter)

    sampleType = parser.add_mutually_exclusive_group(required=True)
    sampleType.add_argument(
         "--combinedSkim", action="store_true", help="xxx"
    )
    sampleType.add_argument(
        "--singleProductions", action="store_true", help="xxx")

    parser.add_argument("-s", "--skims", required=(("--combinedSkims") in sys.argv),
                        nargs=" + ", help="CombinedSkims that you want to register")

    parser.add_argument("-p", "--prods", required=(("--singleProductions") in sys.argv),
                        nargs=" + ", help="Single prods that you want to register")

    parser.add_argument("--printOnly", required=False, help="Print commands without executing them")

    return parser


def update_dataset(meta_fn, execute):

    with open(meta_fn, "r", newline="") as f:
        lines = [line.rstrip() for line in f if "/belle/Data" in line]

    LPN_full_list = [line.split("|")[0] for line in lines]
    LPN_list = [line.split("|")[0] for line in lines if "/hadron/" not in line]
    print("# LPNs to be updated:", len(LPN_list), "(out of", len(LPN_full_list), ")")

    for lpn in LPN_list:
        cmd = f"gb2_ds_searcher_update dataset --lpn {lpn} --general_skim hadron"
        print(cmd)
        if execute:
            subprocess.call(cmd, shell=True)

    print()


def retrieve_info_combinedSkim(skims):  # , prod_fn, meta_fn, failed_meta_fn):
    """
    Given a combined skim, it retrieves the metadata and stores them in a txt file
    """
    prod_list = []
    # meta_fn_list, failed_list = [], []

    for s in skims:
        print("===", s, "===")
        prod_fn = "{0}_DoneProds.txt".format(s)
        prod_all = "{0}_AllProds.txt".format(s)
        # HOTFIX 1Y
        prod_command = "gb2_prod_status --date 1y -g skim -s Done | grep {0} | awk '{{print $1}}' | awk 'NR>0' > {1}".format(
            s, prod_fn
        )
        print("Preparing  list of Done productions:", prod_command)
        subprocess.call(prod_command, shell=True)

        # HOTFIX 1Y
        prod_command_all = "gb2_prod_status --date 1y -g skim -s all"
        prod_command_all += " | grep {0} | grep -v Canceled | awk '{{print $1}}' | awk 'NR>0' > {1}".format(
            s, prod_all)
        subprocess.call(prod_command_all, shell=True)

        problemFound = False
        with open(prod_fn, "r") as file:
            lines = file.readlines()
            line_count = len(lines)
        with open(prod_all, 'r') as file:
            lines_all = file.readlines()
            all_line_count = len(lines_all)

        if line_count != all_line_count:
            problemFound = True
            print(f" === Problem Found! === \n# Done prods (line_count) =! #All prods ({all_line_count}).")
            print(f"Something wrong, maybe the combinedSkim is not fully done. Please check {s} and try again.")

        if not problemFound:
            prod_list.append(prod_fn)
            """

            with open(prod_fn, 'r') as file:
                lines = file.readlines()
            new_lines = [l for l in lines for p in problematic_prods if str(p) not in l]

            print(len(lines), len(new_lines))
            #prod_list.append(prod_fn)
            #

            meta_fn = '{0}_metadata.txt'.format(s)
            failed_meta_fn = '{0}_metadata_failed.txt'.format(s)

            meta_command = 'gb2_prod_show_metadata -i {0} -o {1} -e {2}'.format(prod_fn, meta_fn, failed_meta_fn)
            print('Retrieving metadata for productions: ', meta_command)
            subprocess.call(meta_command, shell=True)

            meta_fn_list.append(meta_fn)
            failed_list.append(failed_meta_fn)
            """

        print("")

    return prod_list
    # return meta_fn_list, failed_list


def remove_problematic_prods(prod_list):
    for prod in prod_list:
        with open(prod, "r") as file:
            lines = file.readlines()

        lines_to_remove = []
        for line in lines:
            for p in problematic_prods:
                if str(p) in line:
                    lines_to_remove.append(line)
        print("# lines to be removed:", len(lines_to_remove), "(out of", len(lines), ")")

        if len(lines_to_remove) != 0:
            new_lines = [line for line in lines if line not in lines_to_remove]

            with open(prod, "w") as file:
                file.writelines(new_lines)
    print("")


def retrieve_metadata_combinedSkim(prod_list):
    meta_fn_list, failed_list = [], []

    for prod_fn in prod_list:
        s = prod_fn.split("_DoneProds.txt")[0]
        print("Retrieving metadata for productions ===> ", s)

        meta_fn = "{0}_metadata.txt".format(s)
        failed_meta_fn = "{0}_metadata_failed.txt".format(s)
        meta_command = "gb2_prod_show_metadata -i {0} -o {1} -e {2}".format(prod_fn, meta_fn, failed_meta_fn)

        print(meta_command)
        subprocess.call(meta_command, shell=True)

        meta_fn_list.append(meta_fn)
        failed_list.append(failed_meta_fn)

    return meta_fn_list, failed_list


def retrieve_info_prods(prods):
    """
    Given a list of productions, it retrieves the metadata and stores them in a txt file
    """
    prod_fn = "SingleProds.txt"

    for i, prod in enumerate(prods):
        prod_command = "gb2_prod_status -g skim -s Done | grep {0} | awk '{{print $1}}' > {1}".format(prod, prod_fn)
        if i != 0:
            prod_command = "gb2_prod_status -g skim -s Done | grep {0} | awk '{{print $1}}' >> {1}".format(prod, prod_fn)
        print("Preparing list for single prods: ", prod_command)
        subprocess.call(prod_command, shell=True)

    problemFound = False
    with open(prod_fn, 'r') as file:
        lines = file.readlines()
        line_count = len(lines)

    if line_count != len(prods):
        print("")
        print(f" === Problem Found! === \n# prods ({len(prods)}) =! #lines in file ({line_count}).")
        print("Please check what you want to register, some of the prods are probably not Done.")
        problemFound = True

    if not problemFound:
        meta_fn = "singleProductions_metadata.txt"
        failed_meta_fn = "singleProductions_metadata_failed.txt"

        meta_command = "gb2_prod_show_metadata -i {0} -o {1} -e {2}".format(prod_fn, meta_fn, failed_meta_fn)
        print("Retrieving metadata for productions:", meta_command)
        subprocess.call(meta_command, shell=True)

    meta_fn_list = [meta_fn]
    failed_list = [failed_meta_fn]

    return meta_fn_list, failed_list


def create_datasets(meta_fn, execute):
    """
    Given a txt file with metadata, it splits them in MC, proc, prod and create datasets
    """

    with open(meta_fn, "r") as file:
        lines = file.readlines()
        header = lines[0]
        MC_lines = [line for line in lines if "/MC/" in line]
        proc_lines = [line for line in lines if "/proc" in line]
        prompt_lines = [line for line in lines if "Prompt" in line or "bucket" in line]
        print("Counting lines in metadata input file:")
        print("MC:", len(MC_lines), "\t proc:", len(proc_lines),  "\t prompt:", len(prompt_lines), "\t total:", len(lines))

    OKcheckOnLines = True
    if len(MC_lines) + len(proc_lines) + len(prompt_lines) != len(lines) - 1:
        OKcheckOnLines = False

    dataset_to_update = []

    if not OKcheckOnLines:
        print(" === Problem Found! ===")
        print("Something went wrong when splitting the input metadata file. Please check")

    else:
        print("")
        # === create datasets for MC ===
        if len(MC_lines) > 0:
            MC_meta_fn = "MC_" + meta_fn
            MC_reg_fn = "MC_REG_" + meta_fn
            MC_failed_reg_fn = "MC_REG_FAIL_" + meta_fn
            with open(MC_meta_fn, "w") as file:
                file.write(header)
                file.writelines(MC_lines)

            MC_create_command = "gb2_ds_searcher_create dataset -i {0} -o {1} -e {2} -bkg BGx1".format(
                MC_meta_fn, MC_reg_fn, MC_failed_reg_fn)
            print("Create dataset for MC \n", MC_create_command)
            if execute:
                subprocess.call(MC_create_command, shell=True)
            print()

        # === create datasets for proc ===
        if len(proc_lines) > 0:
            proc_meta_fn = "proc_" + meta_fn
            proc_reg_fn = "proc_REG_" + meta_fn
            proc_failed_reg_fn = "proc_REG_FAIL_" + meta_fn

            if "HLTHad" in meta_fn:
                dataset_to_update.append(proc_meta_fn)

            with open(proc_meta_fn, 'w') as file:
                file.write(header)
                file.writelines(proc_lines)

            proc_create_command = "gb2_ds_searcher_create dataset -i {0} -o {1} -e {2}".format(
                proc_meta_fn, proc_reg_fn, proc_failed_reg_fn)
            print("Create dataset for proc \n", proc_create_command)
            if execute:
                subprocess.call(proc_create_command, shell=True)
            print()

        # === create datasets for prompt ===
        if len(prompt_lines) > 0:
            prompt_meta_fn = "prompt_" + meta_fn
            prompt_reg_fn = "prompt_REG_" + meta_fn
            prompt_failed_reg_fn = "prompt_REG_FAIL" + meta_fn

            if "HLTHad" in meta_fn:
                dataset_to_update.append(prompt_meta_fn)

            with open(prompt_meta_fn, 'w') as file:
                file.write(header)
                file.writelines(prompt_lines)

            prompt_create_command = "gb2_ds_searcher_create dataset -i {0} -o {1} -e {2}".format(
                prompt_meta_fn, prompt_reg_fn, prompt_failed_reg_fn)
            print("Create dataset for prompt \n", prompt_create_command)
            if execute:
                subprocess.call(prompt_create_command, shell=True)
            print()

    return dataset_to_update


def main():
    parser = get_argument_parser()
    args = parser.parse_args()

    print("")

    execute = True
    if args.printOnly:
        execute = False

    if args.combinedSkim:
        skims = args.skims
        # meta_file_list, failed_list = retrieve_info_combinedSkim(skims)

        prod_list = retrieve_info_combinedSkim(skims)
        remove_problematic_prods(prod_list)
        meta_file_list, failed_list = retrieve_metadata_combinedSkim(prod_list)

    if args.singleProductions:
        prods = args.prods
        meta_file_list, failed_list = retrieve_info_prods(prods)

    print("n metafiles to check:", len(meta_file_list), meta_file_list)
    print("")

    dataset_to_update = []

    for meta_fn in meta_file_list:
        print("===", meta_fn, "===")
        print("")
        ds = create_datasets(meta_fn, execute)
        dataset_to_update += ds
        print("")

    print("=================================================")
    print("")
    print("Dataset to check for updates:", dataset_to_update)
    if len(dataset_to_update) != 0:
        for i, ds in enumerate(dataset_to_update):
            update_dataset(ds, execute)


if __name__ == "__main__":
    main()

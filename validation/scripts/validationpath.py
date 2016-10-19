#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os

folder_name_results = "results"
folder_name_general = "__general__"
file_name_results_json = "revision.json"
file_name_runtimes_dat = "runtimes.dat"


def get_results_runtime_file(output_base_dir):
    """!
    Return the absolute path to the runtimes.dat file
    As there is only the runtimes file of the last iteration stored, this is
    located in the topmost work folder
    """
    return os.path.join(output_base_dir, file_name_runtimes_dat)


def get_results_folder(output_base_dir):
    """!
    Return the absolute path to the results folder
    """
    return os.path.join(output_base_dir, folder_name_results)


def get_results_tag_folder(output_base_dir, tag):
    """!
    Return the absolute path to the results folder for one specific tag
    """
    return os.path.join(get_results_folder(output_base_dir), tag)


def get_results_tag_general_folder(output_base_dir, tag):
    """!
    Return the absolute path to the results folder for one specific tag. In this
    general folder, the common log files will be placed
    """
    return os.path.join(get_results_tag_folder(output_base_dir, tag), folder_name_general)


def get_results_tag_revision_file(output_base_dir, tag):
    """!
    Return the absolute path to the revision.json file for one tag folder
    """
    return os.path.join(get_results_tag_folder(output_base_dir, tag), file_name_results_json)


def get_results_tag_package_folder(output_base_dir, tag, package):
    """!
    Returns the absolute path for a tag and package. This folder will contain
    the ROOT files plots which will be displayed on the validation website
    """
    return os.path.join(
        get_results_tag_folder(output_base_dir, tag),
        package)

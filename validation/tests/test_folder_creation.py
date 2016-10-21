#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import tempfile
import validation
import validationserver
import validationpath
import validationserver

from validationtestutil import check_excecute, check_path_exists


def main():
    """
    Runs two test validations, starts the web server and queries data
    """
    success = True

    rev_to_gen = "test_folder_creation"

    # will create a temporary folder and delete it once this block is left
    with tempfile.TemporaryDirectory() as tmpdir:
        print("Created temporary test folder {}".format(tmpdir))

        expect_results_folder_name = validationpath.get_results_folder(str(tmpdir))

        # switch to this folder
        os.chdir(str(tmpdir))

        validation.execute(tag=rev_to_gen, isTest=True)

        # todo: check if results folder has been created and is filled
        path_to_check = [os.path.join(validationpath.get_results_tag_folder(str(tmpdir), rev_to_gen)),
                         os.path.join(validationpath.get_results_tag_revision_file(str(tmpdir), rev_to_gen)),
                         os.path.join(validationpath.get_results_tag_folder(str(tmpdir), rev_to_gen),
                                      "validation-test", "validationTestPlotsA.py.log"),
                         os.path.join(validationpath.get_results_tag_folder(str(tmpdir), rev_to_gen),
                                      "validation-test", "validationTestPlotsA.root")]

        check_path_exists(path_to_check)

        # run validation server
        # must setup all content in the html folder required
        validationserver.run_server(dryRun=True)

        # check if all files have been copied and the symbolic links properly set
        path_to_check = [os.path.join(str(tmpdir), validationpath.folder_name_html),
                         os.path.join(str(tmpdir), validationpath.folder_name_html, validationpath.folder_name_plots)]
        check_path_exists(path_to_check)

if __name__ == "__main__":
    main()

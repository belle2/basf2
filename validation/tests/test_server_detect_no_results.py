#!/usr/bin/env python3

import sys
import os
import tempfile
import validationpath
import validationserver


def main():
    """
    Test if the validationserver is able to detect if there is no
    results folder available
    """

    # will create a temporary folder and delete it once this block is left
    with tempfile.TemporaryDirectory() as tmpdir:
        print(f"Created temporary test folder {tmpdir}")
        os.chdir(str(tmpdir))

        # run validation server
        fail = False
        try:
            validationserver.run_server(dry_run=True)
            fail = True
        except SystemExit:
            # we expect it to exit
            pass

        if fail:
            sys.exit("did not exit, even though there is no results folder")

    # will create a temporary folder and delete it once this block is left
    with tempfile.TemporaryDirectory() as tmpdir:
        print(f"Created temporary test folder {tmpdir}")
        os.chdir(str(tmpdir))

        # create validation results folders, but still no folders inside
        os.mkdir(validationpath.get_results_folder(os.getcwd()))

        # run validation server
        fail = False
        try:
            validationserver.run_server(dry_run=True)
            fail = True
        except SystemExit:
            # we expect it to exit
            pass

        if fail:
            sys.exit(
                "did not exit, even though there is no folders in "
                "results folder"
            )

    # will create a temporary folder and delete it once this block is left
    with tempfile.TemporaryDirectory() as tmpdir:
        # should work when the folders are present
        print(f"Created temporary test folder {tmpdir}")
        os.chdir(str(tmpdir))

        # create validation results folders, but still no folders inside
        os.mkdir(validationpath.get_results_folder(os.getcwd()))
        os.mkdir(validationpath.get_results_tag_folder(os.getcwd(), "some"))

        # run validation server
        # should not raise any exit exceptions
        validationserver.run_server(dry_run=True)


if __name__ == "__main__":
    main()

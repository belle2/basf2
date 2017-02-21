#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import shutil
from unittest.mock import Mock
import tempfile
import validation
import validationfunctions
import validationpath
import validationserver

from validationtestutil import create_fake_scripts


def main():
    """
    Tests the automated validation script discovery
    """
    success = True

    rev_to_gen = "test_folder_creation"
    all_tags = ["reference", rev_to_gen]

    # will create a temporary folder and delete it once this block is left
    with tempfile.TemporaryDirectory() as tmpdir:
        print("Created temporary test folder {}".format(tmpdir))

        # create a couple of fake validation scripts
        create_fake_scripts(os.path.join(tmpdir, "pkg1", "validation"), "pkg1_a.py")
        create_fake_scripts(os.path.join(tmpdir, "pkg1", "validation"), "pkg1_b.py")
        create_fake_scripts(os.path.join(tmpdir, "pkg1", "validation"), "pkg1_c.py")

        create_fake_scripts(os.path.join(tmpdir, "pkg2", "validation"), "pkg2_b.py")
        create_fake_scripts(os.path.join(tmpdir, "pkg2", "validation"), "pkg2_c.py")

        # this should not be found !
        create_fake_scripts(os.path.join(tmpdir, "some_other_folder", "pkgother", "validation"), "pkgother_b.py")

        # fake the basepath
        basepath = {"local": str(tmpdir)}

        folders = validationfunctions.get_validation_folders(location="local", basepaths=basepath, log=Mock())

        if "pkg1" not in folders or "pkg2" not in folders or "some_other_folder" in folders:
            print("scripts were discovered in the wrong folders")
            sys.exit(1)

        scripts = validationfunctions.scripts_in_dir(folders["pkg1"], Mock(), ".py")

        if len([s for s in scripts if s.endswith("pkg1/validation/pkg1_a.py")]) == 0:
            print("script file was not discovered")
            sys.exit(1)

        print(scripts)

    # test for release and local case
    with tempfile.TemporaryDirectory() as tmpdir:
        print("Created temporary test folder {}".format(tmpdir))

        # create a couple of fake validation scripts
        create_fake_scripts(os.path.join(tmpdir, "pkg1", "validation"), "pkg1_a.py")
        create_fake_scripts(os.path.join(tmpdir, "pkg1", "validation"), "pkg1_b.py")
        create_fake_scripts(os.path.join(tmpdir, "pkg1", "validation"), "pkg1_c.py")

        create_fake_scripts(os.path.join(tmpdir, "pkg2", "validation"), "pkg2_b.py")
        create_fake_scripts(os.path.join(tmpdir, "pkg2", "validation"), "pkg2_c.py")

        # this should not be found !
        create_fake_scripts(os.path.join(tmpdir, "some_other_folder", "pkgother", "validation"), "pkgother_b.py")

        # fake the basepath
        basepath = {"local": str(tmpdir), "central": str(tmpdir)}

        folders = validationfunctions.get_validation_folders(location="central", basepaths=basepath, log=Mock())

        if "pkg1" not in folders or "pkg2" not in folders or "some_other_folder" in folders:
            print("scripts were discovered in the wrong folders")
            sys.exit(1)

    #
if __name__ == "__main__":
    main()

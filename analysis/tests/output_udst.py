#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Tests udst creation with background BGx1 MC"""


import basf2 as b2
import b2test_utils as b2tu
import modularAnalysis as ma
import udst


def create_udst():
    """Create a udst with one event in it."""
    pa = b2.create_path()
    ma.inputMdst("default", b2.find_file("analysis/tests/mdst.root"), path=pa)
    ma.fillParticleList("pi+:all", "", path=pa)
    udst.add_udst_output(path=pa, filename="test.udst.root", particleLists=["pi+:all"])
    b2tu.safe_process(pa, 1)
    return


def test_read_udst():
    """Check that the udst contains the particles storearray"""
    pa = b2.create_path()
    ma.inputMdst("default", "test.udst.root", path=pa)
    ma.printDataStore(path=pa)
    b2tu.safe_process(path=pa)
    return


if __name__ == "__main__":
    with b2tu.clean_working_directory():
        create_udst()
        test_read_udst()

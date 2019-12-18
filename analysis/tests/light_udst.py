#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
import b2test_utils as b2tu
import modularAnalysis as ma

if not b2tu.is_light_build():
    b2.B2RESULT("not a light build")
    b2tu.skip_test(reason="We are not in a light build, so skip the light build tests.")
else:
    b2.B2RESULT("is a light build")


def create_udst():
    """Create a udst with one event in it."""
    pa = b2.create_path()
    ma.inputMdst("default", b2.find_file("analysis/tests/mdst.root"), path=pa)
    ma.fillParticleList("pi+:all", "", path=pa)
    ma.outputUdst("test.udst.root", particleLists=['pi+:all'], path=pa)
    b2tu.safe_process(pa)
    return


def test_read_udst():
    """Check that the udst contains the particles storearray"""
    pa = b2.create_path()
    ma.inputMdst("default", "test.udst.root", path=pa)
    ma.printDataStore(path=pa)
    b2tu.safe_process(pa)
    return


if __name__ == "__main__":
    with b2tu.clean_working_directory():
        create_udst()
        test_read_udst()

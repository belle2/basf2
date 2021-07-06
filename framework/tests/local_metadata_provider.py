#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2  # noqa
from conditions_db.local_metadata import LocalMetadataProvider
from conditions_db import PayloadInformation
from b2test_utils import clean_working_directory
import ROOT
from ROOT import Belle2  # noqa

ROOT.gInterpreter.Declare("#include <framework/database/LocalMetadataProvider.h>")


class LocalMetadataComparison:
    """
    Simple class to obtain all valid payloads matching a given set of names for
    a given experiment and run and compare the python and the C++
    implementation.
    """
    @staticmethod
    def create_cpp_metadataprovider(filename, globaltags):
        """
        Jump through some hoops to create a C++ instance of the local metadata
        provider so that we can check the framework implementation and compare
        it to the python version
        """
        states = ROOT.std.set("string")()
        states.insert("PUBLISHED")
        tags = ROOT.std.vector("string")()
        for tag in globaltags:
            tags.push_back(tag)
        localcpp = Belle2.Conditions.LocalMetadataProvider(filename, states)
        localcpp.setTags(tags)
        return localcpp

    def __init__(self, filename, globaltag, payloads):
        """Create the local metadata providers for the given globaltag and payload names"""
        #: Globaltag
        self._globaltag = globaltag
        #: Payloads
        self._payloads = payloads
        #: Local metadata provider, python version
        self._localpy = LocalMetadataProvider(filename)
        #: Local metadata provider, C++ version
        self._localcpp = self.create_cpp_metadataprovider(filename, [globaltag])

    def check(self, exp, run):
        """Check if the c++ and python version return the same payload revisions
        If not raise a ValueError, if yes return the list of payloads and their
        revision valid for the given exp,run"""
        from_py = [(p.name, p.revision) for p in self._localpy.get_all_iovs(self._globaltag, exp=exp, run=run)
                   if p.name in self._payloads]
        info = ROOT.std.vector("Belle2::Conditions::PayloadMetadata")()
        for name in self._payloads:
            info.push_back(Belle2.Conditions.PayloadMetadata(name, False))
        self._localcpp.getPayloads(exp, run, info)
        from_cpp = [(p.name, p.revision) for p in info
                    if p.revision > 0]
        from_py.sort()
        from_cpp.sort()
        if from_py != from_cpp:
            raise ValueError(f"Payloads not equal: {from_py} != {from_cpp}")
        return from_py


if __name__ == "__main__":
    with clean_working_directory() as dirname:
        # so we can easily create multiple globaltags in a local file to check different iovs
        local = LocalMetadataProvider("testdatabase.sqlite", "overwrite")
        local.add_globaltag(1, "test1", "PUBLISHED", [
            PayloadInformation(1, "test", 1, "", "", "", None, (45, 221, 49, -1))
        ])
        # check a single run validity
        local.add_globaltag(2, "test2", "PUBLISHED", [
            PayloadInformation(1, "test", 1, "", "", "", None, (0, 1, 0, 1))
        ])
        # and unlimited runs in same experiment
        local.add_globaltag(3, "test3", "PUBLISHED", [
            PayloadInformation(1, "test", 1, "", "", "", None, (1, 1, 1, -1))
        ])
        # or also the first run in the next experiment
        local.add_globaltag(4, "test4", "PUBLISHED", [
            PayloadInformation(1, "test", 1, "", "", "", None, (1, 1, 2, 0))
        ])
        # or an unbound iov
        local.add_globaltag(5, "test5", "PUBLISHED", [
            PayloadInformation(1, "test", 1, "", "", "", None, (1, 1, -1, -1))
        ])

        comparer = LocalMetadataComparison("testdatabase.sqlite", "test1", ["test"])
        for exp, run in [(44, 1), (44, 9999), (45, 0), (45, 220), (45, 221), (45, 9999), (46, 0), (46, 221),
                         (49, 0), (49, 9999), (50, 0), (50, 221), (50, 9999)]:
            valid = [("test", 1)] if (45, 221) <= (exp, run) < (50, 0) else []
            assert comparer.check(exp, run) == valid, f"Couldn't find payload for {exp},{run}"

        comparer = LocalMetadataComparison("testdatabase.sqlite", "test2", ["test"])
        for exp, run in [(0, 0), (0, 1), (0, 2), (0, 9999),
                         (1, 0), (1, 1), (1, 2), (1, 9999), (9999, 9999)]:
            valid = [("test", 1)] if (exp, run) == (0, 1) else []
            assert comparer.check(exp, run) == valid, f"Couldn't find payload for {exp},{run}"

        comparer = LocalMetadataComparison("testdatabase.sqlite", "test3", ["test"])
        for exp, run in [(0, 0), (0, 1), (0, 2), (0, 9999),
                         (1, 0), (1, 1), (1, 2), (1, 9999),
                         (243, 0), (245, 9999999), (9999, 9999)]:
            valid = [("test", 1)] if (1, 1) <= (exp, run) < (2, 0) else []
            assert comparer.check(exp, run) == valid, f"Couldn't find payload for {exp},{run}"

        comparer = LocalMetadataComparison("testdatabase.sqlite", "test4", ["test"])
        for exp, run in [(0, 0), (0, 1), (0, 2), (0, 9999),
                         (1, 0), (1, 1), (1, 2), (1, 9999),
                         (243, 0), (245, 9999999), (9999, 9999)]:
            valid = [("test", 1)] if (1, 1) <= (exp, run) <= (2, 0) else []
            assert comparer.check(exp, run) == valid, f"Couldn't find payload for {exp},{run}"

        comparer = LocalMetadataComparison("testdatabase.sqlite", "test5", ["test"])
        for exp, run in [(0, 0), (0, 1), (0, 2), (0, 9999),
                         (1, 0), (1, 1), (1, 2), (1, 9999),
                         (2, 0), (2, 1), (2, 2), (2, 9999),
                         (243, 0), (245, 9999999), (9999, 9999)]:
            valid = [("test", 1)] if (1, 1) <= (exp, run) else []
            assert comparer.check(exp, run) == valid, f"Couldn't find payload for {exp},{run}"

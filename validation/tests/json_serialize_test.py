#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Test serialization of JSON objects.
"""

import unittest
import json
import json_objects


# @cond internal_test


class TestJsonSerialize(unittest.TestCase):
    """
    Test serialization of JSON objects.

    This is usually done as follows:

    1. Create JSON class
    2. Serialize
    3. Deserialize
    4. Check that extracted information is correct
    """

    def test_serialize_comparison(self):

        revs = [
            json_objects.Revision("run1", "iae223auiae", "git_hash1", ""),
            json_objects.Revision("run2", "iuiaehuiaen", "git_hash1", ""),
            json_objects.Revision("arun", "buie223aae", "git_hash1", ""),
        ]

        cplot = json_objects.ComparisonPlot("plot_one")

        cfile = json_objects.ComparisonPlotFile(
            compared_revisions=revs,
            plots=[cplot],
            title="",
            rootfile="",
            package="",
        )

        cp = json_objects.ComparisonPackage("pack1", [cfile])

        comp = json_objects.Comparison(revs, [cp])

        js = json_objects.dumps(comp)
        js_decode = json.loads(js)

        self.assertEqual(1, len(js_decode["packages"]))
        self.assertEqual("arun_run1_run2", js_decode["label"])
        self.assertEqual("pack1", js_decode["packages"][0]["name"])

    def test_serialize_revision_nested(self):
        rr1 = json_objects.Revision("label1", "date", "git_hash1", "black")
        rr2 = json_objects.Revision("label2", "date", "git_hash2", "black")
        rlist = json_objects.Revisions([rr1, rr2])

        js = json_objects.dumps(rlist)
        js_decode = json.loads(js)

        self.assertEqual(2, len(js_decode["revisions"]))
        self.assertEqual("label1", js_decode["revisions"][0]["label"])
        self.assertEqual("label2", js_decode["revisions"][1]["label"])

    def test_comparison_plot_file(self):
        ccp1 = json_objects.ComparisonPlotFile(
            title="title", package="package", rootfile="rootfile"
        )
        ccp2 = json_objects.ComparisonPlotFile("package", "title", "rootfile")

        self.assertEqual(ccp1.title, "title")
        self.assertEqual(ccp1.package, "package")

        self.assertEqual(ccp2.title, "title")
        self.assertEqual(ccp2.package, "package")


if __name__ == "__main__":
    unittest.main()

# @endcond

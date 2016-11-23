#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from ipython_tools import handler
import basf2
import json
import datetime
import json_objects


# @cond internal_test


class TestJsonSerialize(unittest.TestCase):

    def test_serialize_comparison(self):

        revs = [json_objects.Revision("run1", "iae223auiae", "git_hash1", ""),
                json_objects.Revision("run2", "iuiaehuiaen", "git_hash1", ""),
                json_objects.Revision("arun", "buie223aae", "git_hash1", "")]

        cplot = json_objects.ComparisonPlot("plot_one")

        cfile1 = json_objects.ComparisonPlotFile(compared_revisions=revs,
                                                 plots=[cplot],
                                                 title="",
                                                 rootfile="",
                                                 package="")

        cp1 = json_objects.ComparisonPackage("pack1", [cfile1])

        comp = json_objects.Comparison(revs, [cp1])

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
        ccp1 = json_objects.ComparisonPlotFile(title="title", package="package", rootfile="rootfile")
        ccp2 = json_objects.ComparisonPlotFile("package", "title", "rootfile")

        self.assertEqual(ccp1.title, "title")
        self.assertEqual(ccp1.package, "package")

        self.assertEqual(ccp2.title, "title")
        self.assertEqual(ccp2.package, "package")


#     def test_serialize_packages_nested(self):
#
#         plot1 = json_objects.PlotFile("package", "filename", "rootfile", "shortname")
#         plot2 = json_objects.PlotFile("package", "filename", "rootfile", "shortname")
#         plot3 = json_objects.PlotFile("package", "filename", "rootfile", "shortname")
#         plot4 = json_objects.PlotFile("package", "filename", "rootfile", "shortname")
#
#         rr1 = json_objects.Package("package1", [plot1, plot2])
#         rr2 = json_objects.Package("package2", [plot3, plot4])
#         rlist = {"packages": [rr1, rr2]}
#
#         js = json_objects.dumps(rlist)
#         js_decode = json.loads(js)
#
#         self.assertEqual(2, len(js_decode))
#         self.assertEqual("package1", js_decode["packages"][0]["name"])
#         self.assertEqual(2, len(js_decode["packages"][1]["plotfiles"]))
#         self.assertEqual("filename", js_decode["packages"][1]["plotfiles"][0]["filename"])

if __name__ == "__main__":
    unittest.main()

# @endcond

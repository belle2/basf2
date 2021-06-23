#!/usr/bin/env python3

# Test that globaltags are forwarded correclty to the output file even if the
# database isn't used (BII-5790)

import basf2
import b2test_utils
from ROOT import Belle2
import json
import unittest


class TestGTForwarding(unittest.TestCase):
    """Test that we forward globaltags correctly"""
    def test_it(self):
        """Only one test here ..."""
        inputfile = b2test_utils.require_file("mdst12.root", "validation", self)
        metadata = basf2.get_file_metadata(inputfile)
        inputtags = metadata.getDatabaseGlobalTag()
        self.assertNotEqual(inputtags, "")

        with b2test_utils.clean_working_directory():
            basf2.conditions.globaltags = ["A", "B", "C", "A", "B", "C", "C", "A"]
            Belle2.MetadataService.Instance().setJsonFileName("info.json")
            p = basf2.Path()
            p.add_module("RootInput", inputFileName=inputfile)
            p.add_module("RootOutput", outputFileName="test.root")
            basf2.process(p, 3)
            with open("info.json") as jobinfo:
                data = json.load(jobinfo)

        self.assertIn('output_files', data, "Expecting output file info")
        self.assertEqual(len(data['output_files']), 1, "Expecting exactly one output file")
        fileinfo = data['output_files'][0]
        # FIXME: Once we get rid of the legacy ip globaltag this will get nicer
        self.assertEqual(fileinfo['metadata']['globalTag'], f"A,B,C,{inputtags},Legacy_IP_Information",
                         "Globaltags not forwarded correctly")
        self.assertEqual(fileinfo['stats']['events'], 3, "Number of events is wrong")


if __name__ == '__main__':
    unittest.main()

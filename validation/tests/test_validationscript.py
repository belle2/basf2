#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import unittest
import tempfile
import validationscript

call_iteration = 0


class ValidationScriptTest(unittest.TestCase):

    """
    Various test cases for the Script class
    """

    def test_parse_header(self):
        """
        Test if the xml header in validation scripts are parsed properly
        """
        with tempfile.NamedTemporaryFile() as tf:
            tf.write(
                b"#!/usr/bin/env python3\n"
                b"# -*- coding: utf-8 -*-\n"
                b'"""\n'
                b"<header>\n"
                b"<interval>release</interval>"
                b"<output>EvtGenSim.root</output>\n"
                b"<contact>Thomas Kuhr thomas.kuhr@lmu.de</contact>\n"
                b"<description>description_text</description>\n"
                b"</header>\n"
                b'"""\n'
            )

            # flush file content, so it can be read by the Script class used
            # below
            tf.flush()

            script = validationscript.Script(tf.name, "package", None)
            script.load_header()

            self.assertEqual("description_text", script.description)
            self.assertEqual(
                "Thomas Kuhr thomas.kuhr@lmu.de", script.contact[0]
            )
            self.assertEqual(1, len(script.output_files))
            self.assertEqual("EvtGenSim.root", script.output_files[0])
            self.assertEqual("release", script.interval)


if __name__ == "__main__":
    unittest.main()

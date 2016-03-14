#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import metaoptions
import ROOT
import random
import math
import tempfile
import validationscript
import validation

call_iteration = 0


class ValidationTest(unittest.TestCase):

    """
    Various test cases for the validation module
    """

    def test_interval_selection(self):
        """
        Test if the interval selection works
        """
        intervalSel = validation.IntervalSelector(["release", " nightly"])

        with tempfile.NamedTemporaryFile() as tf:
            tf.write(b'#!/usr/bin/env python3\n'
                     b'# -*- coding: utf-8 -*-\n'
                     b'"""\n'
                     b'<header>\n'
                     b'<interval>release</interval>'
                     b'<output>EvtGenSim.root</output>\n'
                     b'<contact>tkuhr</contact>\n'
                     b'<description>description_text</description>\n'
                     b'</header>\n'
                     b'"""\n')

            # flush file content, so it can be read by the Script class used
            # below
            tf.flush()

            script = validationscript.Script(tf.name, "package", None)
            script.load_header()
            self.assertTrue(intervalSel.in_interval(script))

        with tempfile.NamedTemporaryFile() as tf:
            tf.write(b'#!/usr/bin/env python3\n'
                     b'# -*- coding: utf-8 -*-\n'
                     b'"""\n'
                     b'<header>\n'
                     b'<interval>nightly</interval>'
                     b'<output>EvtGenSim.root</output>\n'
                     b'<contact>tkuhr</contact>\n'
                     b'<description>description_text</description>\n'
                     b'</header>\n'
                     b'"""\n')

            # flush file content, so it can be read by the Script class used
            # below
            tf.flush()

            script = validationscript.Script(tf.name, "package", None)
            script.load_header()
            self.assertTrue(intervalSel.in_interval(script))

    def test_interval_selection_default(self):
        """
        Test if the interval selection works if there is no
        interval setting in the validation header
        """
        with tempfile.NamedTemporaryFile() as tf:
            tf.write(b'#!/usr/bin/env python3\n'
                     b'# -*- coding: utf-8 -*-\n'
                     b'"""\n'
                     b'<header>\n'
                     b'<output>EvtGenSim.root</output>\n'
                     b'<contact>tkuhr</contact>\n'
                     b'<description>description_text</description>\n'
                     b'</header>\n'
                     b'"""\n')

            # flush file content, so it can be read by the Script class used
            # below
            tf.flush()

            script = validationscript.Script(tf.name, "package", None)
            script.load_header()

            intervalSel = validation.IntervalSelector(["release", " nightly"])
            self.assertTrue(intervalSel.in_interval(script))

    def test_meta_option_parser(self):
        """
        Test if the meta options parsers behaves nice
        """
        p = metaoptions.MetaOptionParser(["expert", "pvalue-warn=0.9", "pvalue-error=0.4"])

        self.assertEqual(0.9, p.pvalue_warn())
        self.assertEqual(0.4, p.pvalue_error())
        self.assertTrue(p.has_option("expert"))
        self.assertFalse(p.has_option("not is list"))

        p = metaoptions.MetaOptionParser(["expert", "pvalue-warn="])

        self.assertEqual(None, p.pvalue_warn())
        self.assertEqual(None, p.pvalue_error())

if __name__ == "__main__":
    unittest.main()

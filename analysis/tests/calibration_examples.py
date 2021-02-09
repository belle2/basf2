#!/usr/bin/env python3
# -- analysis/tests/calibration_examples.py

import b2test_utils as b2tu
import b2test_utils_analysis as b2tua

#: extend the examples test class to actually run over the calibration examples


class ExamplesTest(b2tua.ExamplesTest):
    def test_calibration_examples(self):
        """
        Test supported calibration examples.
        """
        b2tu.skip_test_if_light(self)  # calibration dataobjects not supported in light releases

        # list of the broken examples (to be removed when they are individually fixed)
        broken_cal_egs = ['B2CAL901-cDSTECLTRG.py'  # BII-4276
                          ]

        self._test_examples_dir('analysis/examples/calibration/', broken_cal_egs)


if __name__ == '__main__':
    with b2tu.clean_working_directory():
        b2tua.unittest.main()

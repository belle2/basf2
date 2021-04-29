#!/usr/bin/env python3
# -- analysis/tests/mva_examples.py

import b2test_utils as b2tu
import b2test_utils_analysis as b2tua

#: extend the examples test class to actually run over the mva examples:


class ExamplesTest(b2tua.ExamplesTest):
    def test_mva_examples(self):
        """
        Test supported mva examples.
        """
        # B2A714 is a MVA model description and not a standalone example script
        excluded_python_file = ['B2A714-DeepContinuumSuppression_MVAModel.py']

        self._test_examples_dir('analysis/examples/mva/', excluded_python_file)


if __name__ == '__main__':
    with b2tu.clean_working_directory():
        b2tua.unittest.main()

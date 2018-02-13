Testing Tools
+++++++++++++

These tools are used to run the continous integration tests. We have unittests and script tests.

.. index::
    single: tools; b2test-units
    single: testing; b2test-units

.. _b2test-units:

``b2test-units``: Run unittests
-------------------------------

This program runs all unittests of the Belle2 Software. It is based on `Google
Test <https://github.com/google/googletest>`_. Usually it's enough to call it
without arguments. Here we just document the most useful arguments, please use
``--help`` to get a more detailed list of avialable options.

.. rubric:: Optional Arguments

--help                  Show a detailed list of all options
--gtest_list_tests      List all tests instead of running them
--gtest_filter=PATTERN  Run only the tests whose name matches the pattern
--gtest_output_xml=FILENAME
                        Write the test results into a JUnit compatible XML file


.. index::
    single: tools; b2test-scripts
    single: testing; b2test-scripts

.. _b2test-scripts:

``b2test-scripts``: Run framework tests
---------------------------------------

.. argparse::
    :filename: framework/tools/b2test-scripts
    :func: get_argument_parser
    :prog: b2test-scripts
    :nodefaultconst:
    :nogroupsections:

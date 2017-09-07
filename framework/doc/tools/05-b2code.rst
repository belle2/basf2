Tools to help with debugging and checking code quality
++++++++++++++++++++++++++++++++++++++++++++++++++++++

``b2code-memoryusage``: Show memory usage of a program
------------------------------------------------------

.. argparse::
    :filename: framework/tools/b2code-memoryusage
    :func: get_argument_parser
    :prog: b2code-memoryusage
    :nodefault:
    :nogroupsections:

``b2code-doxygen-warnings``: Show warnings when running doxygen
---------------------------------------------------------------

This script will try to show all doxygen warnings about undocumented items
similar to the development build. To run it on single files or directories just
supply them as arguments::

    b2code-doxygen-warnings [<directory>...] [<filename>...]

To run on the full release directory run it without any arguments::

    b2code-doxygen-warnings

``b2code-parallel_processing-benchmark``: Measure multi-core performance
------------------------------------------------------------------------

.. argparse::
    :filename: framework/tools/b2code-parallel_processing-benchmark
    :func: get_argument_parser
    :prog: b2code-parallel_processing-benchmark
    :nodefault:
    :nogroupsections:

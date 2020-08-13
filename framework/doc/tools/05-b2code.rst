Tools to help with debugging and checking code quality
++++++++++++++++++++++++++++++++++++++++++++++++++++++

``b2code-module``: Create a source code template for a module
-------------------------------------------------------------

.. argparse::
    :filename: framework/tools/b2code-module
    :func: get_argument_parser
    :prog: b2code-module
    :nodefault:
    :nogroupsections:

``b2code-memoryusage``: Show memory usage of a program
------------------------------------------------------

.. argparse::
    :filename: framework/tools/b2code-memoryusage
    :func: get_argument_parser
    :prog: b2code-memoryusage
    :nodefault:
    :nogroupsections:

``b2code-findsymbol``: Look for a given C++ symbol
--------------------------------------------------

This script will try to find all libraries which contain a given C++ symbol.
The intended use is to find out which library to add to the ``SConscript`` file
as a dependency in case of an unresolved symbol error. Just run it with the
symbol name as an argument::

    b2code-findsymbol <symbolname>

The symbol name can be any valid grep pattern. It is advisable to quote the
symbol name in single quotes to prevent the shell to interpret special
characters. For example to look for ``Belle2::MCParticle::Class()`` one would
use::

    b2code-findsymbold 'Belle2::MCParticle::Class()'

It will print all the symbols found prefixed with the library name and a
condensed list of all libraries containing the match at the end. When adding
libraries to a ``SConscript`` please remove the ``lib`` prefix and ``.so``
extension.


``b2code-doxygen-warnings``: Show warnings when running doxygen
---------------------------------------------------------------

This script will try to show all doxygen warnings about undocumented items
similar to the development build. To run it on single files or directories just
supply them as arguments::

    b2code-doxygen-warnings [<directory>...] [<filename>...]

To run on the full release directory run it without any arguments::

    b2code-doxygen-warnings


``b2code-classversion-check``: Perform sanity checks on ROOT class versions
---------------------------------------------------------------------------

.. argparse::
    :filename: framework/tools/b2code-classversion-check
    :func: get_argument_parser
    :prog: b2code-classversion-check
    :nodefault:
    :nogroupsections:


``b2code-classversion-update``: Update linkdef information for sanity checks
----------------------------------------------------------------------------

.. argparse::
    :filename: framework/tools/b2code-classversion-update
    :func: get_argument_parser
    :prog: b2code-classversion-update
    :nodefault:
    :nogroupsections:



``b2code-cppcheck``: Run cppcheck static analyzer
-------------------------------------------------

This script will run cppcheck with the appropriate arguments to check for
warnings from cppcheck as shown on the development build.

To run it on single files or directories just supply them as arguments::

    b2code-cppcheck [<directory>...] [<filename>...]

To run on the full release directory run it without any arguments::

    b2code-cppcheck

Additional options can be passed to cppcheck as well, for example -j to run
cppcheck in parallel::

    b2code-cppcheck -j50 framework

However in this case a directory to check has to be provided


``b2code-parallel_processing-benchmark``: Measure multi-core performance
------------------------------------------------------------------------

.. argparse::
    :filename: framework/tools/b2code-parallel_processing-benchmark
    :func: get_argument_parser
    :prog: b2code-parallel_processing-benchmark
    :nodefault:
    :nogroupsections:

``b2code-sphinx-build``: Build the sphinx documentation
-------------------------------------------------------

This script will run sphinx with the appropriate arguments to build the user
documentation and optionally only show the warnings.

.. rubric:: Optional Arguments

--light           Create documentation only for the light release, that is only
                  for the packages included in a light release build.
-t TARGET, --target TARGET
                  Documentation format to generate. If not given the default
                  will be "html". Available options are "html", "latex", "json"
-h, --help        Show this help message
-o, --output-dir  Where to put the documentation. If not given the default is
                  the build directory in the software/release directory

All other options will be forwarded to ``sphinx-build``


``b2code-sphinx-warnings``: Show all the warnings when building the documentation
---------------------------------------------------------------------------------

This script will create the sphinx documentation and check whether there are any warnings.
All extra arguments are forwarded to ``b2code-sphinx-build``.

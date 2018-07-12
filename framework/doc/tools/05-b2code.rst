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

``b2code-parallel_processing-benchmark``: Measure multi-core performance
------------------------------------------------------------------------

.. argparse::
    :filename: framework/tools/b2code-parallel_processing-benchmark
    :func: get_argument_parser
    :prog: b2code-parallel_processing-benchmark
    :nodefault:
    :nogroupsections:

Tools which will provide you help
+++++++++++++++++++++++++++++++++

.. _b2help-modules:

``b2help-modules``: List available `basf2` modules and ask about their details
------------------------------------------------------------------------------

.. argparse::
    :filename: framework/tools/b2help-modules
    :func: get_argument_parser
    :prog: b2help-modules
    :nodefault:
    :nogroupsections:


.. _b2help-particles:

``b2help-particles``: List particles used in the event generator
----------------------------------------------------------------

.. argparse::
    :filename: framework/tools/b2help-particles
    :func: get_argument_parser
    :prog: b2help-particles
    :nodefault:
    :nogroupsections:


.. _b2help-recommendation:

``b2help-recommendation``: Tool to print analysis recommendations
-----------------------------------------------------------------

.. argparse::
    :filename: framework/tools/b2help-recommendation
    :func: get_argument_parser
    :prog: b2help-recommendation
    :nodefault:
    :nogroupsections:

.. _b2help-variables:

``b2help-variables``: List available variables in the analysis package
----------------------------------------------------------------------

.. argparse::
    :filename: analysis/tools/b2help-variables
    :func: get_argument_parser
    :prog: b2help-variables
    :nodefault:
    :nogroupsections:


.. _b2help-print-fei-mode-names:

``b2help-print-fei-mode-names``: List available FEI modes for a given particle
------------------------------------------------------------------------------

.. argparse::
    :filename: analysis/tools/b2help-print-fei-mode-names
    :func: get_argument_parser
    :prog: b2help-print-fei-mode-names
    :nodefault:
    :nogroupsections:


.. include:: ../../../decfiles/doc/decfiles-tools.rstinclude


.. _b2help-print-mcparticles:

``b2help-print-mcparticles``: Print the MCParticles in the input mdst file
--------------------------------------------------------------------------

This command prints the MCParticles of the input mdst file and outputs an ntuple including ``__MCDecayString__``.
Default output file name is ``ntuple.root``.

You can use any of `basf2`'s options, e.g. ``-o``, ``-n``, and ``--skip-events``.
Specifically, ``-i`` is required to select the input mdst file.

::

    usage: b2help-print-mcparticles [-h] [-i INPUT]

.. rubric:: Required Arguments

-i INPUT, --input INPUT  input mdst file name

.. rubric:: Optional Arguments

-h, --help               show this help message and exit


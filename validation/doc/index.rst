Tools for Physics Validation of the Software
++++++++++++++++++++++++++++++++++++++++++++

The ``validation`` package is used to validate the physics performance of
the software. As the software is constantly developed further, algorithms are
changed and we want to make sure that they don't perform worse than before
(e.g. lower reconstruction efficiencies or slower runtime) and that they work
as intended.

To that end, every software package has a ``validation`` subdirectory that
contains a number of simple steering files. The ``validation`` package collects
these steering files and runs them. The resulting output ``*.root`` files can then be compared
across software versions ("revisions") to spot performance degradations and other problems.

To provide a simple user interface, the ``validation`` package contains a small webserver,
that can also be run locally to show the comparison plots.

The validation framework is also run centrally on a nightly basis.
The results are shown at `https://validation.belle2.org/ <https://validation.belle2.org/>`_.

Command line utilities
======================

``b2validation``: Run Physics Validation
----------------------------------------

This program is used to run the full physics validation suite

.. argparse::
    :module: validationfunctions
    :func: get_argument_parser
    :prog: b2validation
    :nodefault:
    :nogroupsections:
    :absolutecommand:

``b2validation-plots``: Compare and plot a selection of revisions
-----------------------------------------------------------------

Select several different software versions (revisions) for which output files
were generated using ``b2validation`` and generate comparison plots:

.. argparse::
    :filename: validation/tools/b2validation-plots
    :func: get_argument_parser
    :prog: b2validation-plots
    :nodefault:
    :nogroupsections:
    :absolutecommand:

``b2validation-server``: Run Webserver Displaying the Physics Validation Results
--------------------------------------------------------------------------------

With ``b2validation-server`` you can run a local webserver to display the
results of a physics validation performed with ``b2validation``.

.. argparse::
    :module: validationserver
    :func: get_argument_parser
    :prog: b2validation-server
    :nodefault:
    :nogroupsections:
    :absolutecommand:

``b2validation-bisect``: Find commits which introduced validation degradation
-----------------------------------------------------------------------------

.. argparse::
    :filename: validation/tools/b2validation-bisect
    :func: get_argument_parser
    :prog: b2validation-bisect
    :nodefault:
    :nogroupsections:
    :absolutecommand:

``b2validation-revisions``: Extract quantities from each revision
-----------------------------------------------------------------

.. argparse::
    :filename: validation/tools/b2validation-revisions
    :func: get_argument_parser
    :prog: b2validation-bisect
    :nodefault:
    :nogroupsections:
    :absolutecommand:

Convenience Functions
=====================

In order to simplify the generation of validation histograms in your basf2
steering file, you can use the following:

.. autofunction:: validation_tools.metadata.create_validation_histograms

You can also add metadata to an existing ROOT object using

.. autofunction:: validation_tools.metadata.validation_metadata_set

or add metadata to an already written ROOT file:

.. autofunction:: validation_tools.metadata.validation_metadata_update
.. autofunction:: validation_tools.metadata.file_description_set

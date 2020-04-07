.. _skim:

=====
Skims
=====

.. warning::
        Skim package sphinx documentation is currently in progress. What is here is correct, but you should take a look at the `confluence page <https://confluence.desy.de/display/BI/Skimming+Homepage>`_ for more information about more skims than is listed here.


The skim package is a collection of high-level analysis scripts that reduce the data set to a manageable size by applying a simple selection.
The input to a skim are `mdst` files of processed data. The output is a so-called udst (micro-dst) which actually contains more information but fewer events.
That is: a udst file contains the same as an `mdst` but with the addition of `ParticleList`_ s of candidates build using the `modularAnalysis` tools.

.. _ParticleList: https://b2-master.belle2.org/software/development/classBelle2_1_1ParticleList.html

To get information about existing skim, ask the current skim group liaison.


Systematics skims
-----------------

.. toctree::
    :maxdepth: 2
    :glob:

    01-systematics.rst

Physics skims
-------------

.. toctree::
    :maxdepth: 3
    :glob:

    02-physics.rst

Standard skim lists
-------------------

.. toctree::
    :maxdepth: 2
    :glob:

    03-lists.rst

Writing, testing, and running skims
-----------------------------------

.. toctree::
    :maxdepth: 2
    :glob:

    04-experts.rst


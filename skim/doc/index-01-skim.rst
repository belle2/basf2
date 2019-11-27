.. _skim:

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

.. automodule:: skim.systematics
    :members:


Physics skims
-------------

:math:`B\to\textrm{charm}` skims
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: skim.btocharm
    :members:

:math:`B\to\textrm{charmless}` skims
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: skim.btocharmless
    :members:

Charm physics skims
~~~~~~~~~~~~~~~~~~~

.. automodule:: skim.charm
    :members:

.. Commented out, because this module no longer exists
   Charmonium skims
   ~~~~~~~~~~~~~~~~

   .. automodule:: skim.charmonium
       :members:

Dark sector skims
~~~~~~~~~~~~~~~~~

.. automodule:: skim.dark
    :members:

Full event interpretation skims
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: skim.fei
    :members:
    :undoc-members:

Leptonic skims
~~~~~~~~~~~~~~

.. automodule:: skim.leptonic
    :members:
    :undoc-members:
    
Quarkonium skims
~~~~~~~~~~~~~~~~

.. automodule:: skim.quarkonium
    :members:

Radiative and electroweak penguins skims
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: skim.ewp
    :members:

Semileptonic skims
~~~~~~~~~~~~~~~~~~

.. automodule:: skim.semileptonic
    :members:
    :undoc-members:

Time-dependent CP-violation (TDCPV) analysis skims
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: skim.tcpv
    :members:

Tau-pair skims
~~~~~~~~~~~~~~~~~

.. automodule:: skim.taupair
    :members:


For skim liaisons and developers
--------------------------------

Some functions and tools are for the use of skim liaisons and developers.

.. tip:: If you are only interested in the selection criteria this section is probably not relevant for you.


.. automodule:: skim.registry
    :members:
 

.. automodule:: skimExpertFunctions
    :members:
    :undoc-members:

Skim performance statistics
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Two command-line tools are provided to test the performance of a skim. They are available in ``skim/tools/stats/``.

To test a skim, first check that the skim is listed in `skim.registry` in the currently setup basf2 environment, and that its name in the registry matches the name of the standalone steering file in ``skim/standalone/`` or ``skim/combined/``. This is how ``submitTestSkims.py`` and ``printSkimStats.py`` get their lists of valid options and find the steering files to run.

Run ``submitTestSkims.py``, which will submit small skim jobs on test files of MC and data using ``bsub``. For example,

.. code-block:: sh

    ./submitTestSkims.py -s LeptonicUntagged SLUntagged
    ./submitTestSkims.py -c BtoCharm
    ./submitTestSkims.py -s BtoXgamma -c feiHadronicCombined

Once all of the submitted jobs have completed successfully, then run ``printSkimStats.py``.

.. code-block:: sh

    ./printSkimStats.py -s LeptonicUntagged
    ./printSkimStats.py -c BtoCharm
    ./printSkimStats.py -s BtoXgamma -c feiHadronicCombined

This will read the output files of the test jobs, and produce tables of statistics in the following three formats:

* A subset of the statistics printed to the screen per-skim. This output is for display only.

* A text file ``skimStats_confluence.txt`` is written, in which the statistics are formatted as Confluence wiki markup tables. These tables can be copied directly onto a Confluence page by editing the page, selecting ``Insert more content`` from the toolbar, selecting ``Markup`` from the drop-down menu, and then pasting the content of the text file into the markup editor which appears. Confluence will then format the tables and headings. The markup editor can also be accessed via ``ctrl-shift-D`` (``cmd-shift-D``).

* All statistics produced are printed to a JSON file ``skimStats.json``, indexed by skim, statistic, and sample label. This file is to be used by grid production tools.

``submitTestSkims.py``: Run skim scripts on test samples
.........................................................

.. argparse::
   :filename: skim/tools/stats/submitTestSkims.py
   :func: getArgumentParser
   :prog: ./submitTestSkims.py
   :nodefaultconst:
   :nogroupsections:

   .. note::
      Please run these skim tests on KEKCC, so that the estimates for CPU time are directly
      comparable to one another.


``printSkimStats.py``: Print tables of performance statistics
..............................................................

.. argparse::
   :filename: skim/tools/stats/printSkimStats.py
   :func: getArgumentParser
   :prog: ./printSkimStats.py
   :nodefaultconst:
   :nogroupsections:

   .. note::
      This tool uses the third-party package `tabulate <https://pypi.org/project/tabulate>`_, which
      can be installed via ``pip``.

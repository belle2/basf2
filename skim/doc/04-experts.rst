Some functions and tools are for the use of skim liaisons and developers. A Jupyter notebook skimming tutorial can be found within the basf2 ./skim/tutorial directory.

.. tip:: If you are only interested in the selection criteria this section is probably not relevant for you.


.. automodule:: skim.registry
    :members:
    :undoc-members: 

.. automodule:: skimExpertFunctions
    :members:
    :undoc-members:

.. _testing-skims:

Testing skim performance
~~~~~~~~~~~~~~~~~~~~~~~~~~~

When skims are developed, it is important to test the performance of the skim on a data and on a range of background MC samples. Two command-line tools are provided in the skim package to aid in this: ``b2skim-stats-submit`` and ``b2skim-stats-print``. They are available in the ``PATH`` after setting up the ``basf2`` environment after calling `b2setup`. The former submits a series of test jobs for a skim on data and MC samples, and the latter uses the output files of the jobs to calculate performance statistics for each sample including retention rate, CPU time, and uDST size per event. ``b2skim-stats-print`` also provides estimates for combined MC samples, where the statistics are weighted by the cross-section of each background process.

First run ``b2skim-stats-submit``, which will submit small skim jobs on test files of MC and data using ``bsub``. For example,

.. code-block:: sh

    b2skim-stats-submit -s LeptonicUntagged SLUntagged

Once all of the submitted jobs have completed successfully, then run ``b2skim-stats-print``.

.. code-block:: sh

    b2skim-stats-print -s LeptonicUntagged SLUntagged

This will read the output files of the test jobs, and produce tables of statistics in the following three formats:

* A subset of the statistics printed to the screen per-skim. This output is for display only.

* A text file ``confluenceTables.txt`` is written, in which the statistics are formatted as Confluence wiki markup tables. These tables can be copied directly onto a Confluence page by editing the page, selecting ``Insert more content`` from the toolbar, selecting ``Markup`` from the drop-down menu, and then pasting the content of the text file into the markup editor which appears. Confluence will then format the tables and headings. The markup editor can also be accessed via ``ctrl-shift-D`` (``cmd-shift-D``).

* All statistics produced are printed to a JSON file ``skimStats.json``, indexed by skim, statistic, and sample label. This file is to be used by grid production tools.

.. tip::
   To test your own newly-developed skim, you must do the following things in your current setup of ``basf2``:

   1. Add your skim to the `skim.registry.Registry`. This should be an entry of the form ``(SkimCode, ParentModule, SkimName)``.

   2. Put your skim steering file in ``skim/standalone/``, with a name of the form ``{SkimName}_Skim_Standalone.py``. This skim name must match what you wrote in the registry.

   3. If your skim relies on any particle list modules which you have added or modified, make sure these are included in ``skim/scripts/skim/``.

   If these three things are done, then the stats tools will be able to find and run your skim.


``b2skim-stats-submit``: Run skim scripts on test samples
.........................................................

.. argparse::
   :filename: skim/tools/b2skim-stats-submit
   :func: getArgumentParser
   :prog: b2skim-stats-submit
   :nodefaultconst:
   :nogroupsections:

   .. note::
      Please run these skim tests on KEKCC, so that the estimates for CPU time are directly
      comparable to one another.


``b2skim-stats-print``: Print tables of performance statistics
..............................................................

.. argparse::
   :filename: skim/tools/b2skim-stats-print
   :func: getArgumentParser
   :prog: b2skim-stats-print
   :nodefaultconst:
   :nogroupsections:

   .. note::
      This tool uses the third-party package `tabulate <https://pypi.org/project/tabulate>`_, which
      can be installed via ``pip``.


``b2skim-prod-json``: Produce grid production requests
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. argparse::
   :filename: skim/tools/b2skim-prod-json
   :func: get_argument_parser
   :prog: b2skim-prod-json
   :nodefaultconst:
   :nogroupsections:

.. tip:: 
    The functions and tools documented here are intended for skim liaisons and developers. If you are only interested in the selection criteria, then this section is probably not relevant for you.

.. _writing-skims:

Writing a skim
~~~~~~~~~~~~~~

In the skim package, skims are defined via the `BaseSkim` class. The skim package is organised around this for the following reasons:

* this keeps the package organised, with every skim being defined in a predictable way,
* this allows the skims to be located by standard helper tools such as :ref:`b2skim-run<skim-running>` and :ref:`b2skim-stats-print<b2skim-stats-print>`, and
* skims must be combined with other skims to reduce the number of grid job submissions, and the `CombinedSkim` class is written to combined objects of type `BaseSkim`.

To write a new skim, please follow these steps:

1. Start by defining a class which inherits from `BaseSkim` and give it the name of your skim. Put the class in an appropriate skim module for your working group. For example, the skim ``DarkSinglePhoton`` belongs in ``skim/scripts/skim/dark.py``, and begins with the following definition:

   .. code-block:: python

       class DarkSinglePhoton(BaseSkim):
           # docstring here explaining reconstructed decay modes and applied cuts.

2. *[Mandatory]* Tell us about your skim by setting the following attributes:

   * ``__description__``: one-line summary describing the purpose of your skim.
   * ``__category__``: a list of category keywords.
   * ``__authors__``: list of skim authors.
   * ``__contact__``: the name and contact email of the skim liaison responsible for this skim.

   `BaseSkim` requires you to set these attributes in each subclass. Once these are set, we can we add a lovely auto-generated header to the documentation of the skim by using the `fancy_skim_header` decorator.

   .. code-block:: python

       @fancy_skim_header
       class DarkSinglePhoton(BaseSkim):
           # docstring here describing your skim, and explaining cuts.

   This header will appear as a "Note" block at the top of your skim class on Sphinx, and will also appear at the top of the help function in an interactive Python session:

       >>> from skim.foo import MySkim
       >>> help(MySkim)

   .. tip::
       If your skim does not define ``__description__``, ``__category__``, ``__authors__``, ``__contact__``, ``RequiredStandardLists`` or ``build_lists``, then you will see an error message like:

       ::

           TypeError: Can't instantiate abstract class SinglePhotonDark with abstract methods __authors__

       This can be fixed by defining these required attributes and methods.

3. *[Mandatory]* Specify all required particle lists in the attribute ``RequiredStandardLists``. If you don't require any standard lists, then you can set this to ``None``. See the documentation of this attribute for instructions on how to do this for your skim.

4. If any further setup is required, then override the ``additional_setup`` method.   

5. *[Mandatory]* Define all cuts by overriding ``build_lists``. Before the end of the ``build_lists`` method, the attribute ``SkimLists`` must be set to a list of skim list names.

6. If any modules are producing too much noise, then override the attribute ``NoisyModules`` as a list of those modules, and their output will be set to only print error-level messages.

7. Add your skim to the registry, with an appropriate skim code (see :ref:`Skim Registry<skim-registry>`).

With all of these steps followed, you will now be able to run your skim using the :ref:`skim command line tools<skim-running>`. To make sure that you skim does what you expect, and is feasible to put into production, please also complete the following steps:

8. Test your skim! The primary point of skims is to be run on the grid, so you want to be sure that the retention rate and processing time are low enough to make this process smooth.

   The skim package contains a set of tools to make this straightforward for you. See `Testing skim performance`_ for more details.

9. Define validation histograms for your skim by overriding the method ``validation_histograms``. Please see the source code of various skims for examples of how to do this.

Calling an instance of a skim class will run the particle list loaders, setup function, list builder function, and uDST output function. So a minimal skim steering file might consist of the following:

.. code-block:: python

    import basf2 as b2
    import modularAnalysis as ma
    from skim.foo import MySkim

    path = b2.Path()
    ma.inputMdstList("default", [], path=path)
    skim = MySkim()
    skim(path)  # __call__ method loads standard lists, creates skim lists, and saves to uDST
    path.process()


.. _skim-running:

Running a skim
~~~~~~~~~~~~~~

In the skim package, there is a command-line tool available for running skims, documented below. These take a skim name as a command line argument, and run the code defined in the corresponding subclass of `BaseSkim`.

.. tip::
   This tool completely supplants the ``<SkimName>_Skim_Standalone.py`` steering files from
   previous versions of basf2. The ``standalone/`` and ``combined/`` directories no longer exist in
   the skim package from version-05-00-00 onwards.

.. argparse::
   :filename: skim/tools/b2skim-run
   :func: get_argument_parser
   :prog: b2skim-run
   :nodefaultconst:
   :nogroupsections:


Skim tutorial
~~~~~~~~~~~~~

A Jupyter notebook skimming tutorial can be found in ``skim/tutorial/Skimming_Tutorial.ipynb`` in basf2.

.. _skim-registry:

Skim registry
~~~~~~~~~~~~~

All skims must be registered and encoded by the relevant skim liaison. Registering a skim is as simple as adding it to the list in `skim.registry.RegisteredSkims` as an entry of the form ``(SkimCode, ParentModule, SkimName)``.

The skim numbering convention is defined on the `Confluence skim page`_.

.. _Confluence skim page: https://confluence.desy.de/x/URdYBQ

.. automodule:: skim.registry
    :members:
    :undoc-members:

.. _skim-expert-functions:

Utility functions for skim experts
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The module ``skimExpertFunctions`` contains helper functions to perform common tasks relating to skims. Importantly, it contains the `skimExpertFunctions.BaseSkim` class, which is how skims are defined.

.. automodule:: skimExpertFunctions
    :members:
    :undoc-members:

.. _testing-skims:

Testing skim performance
~~~~~~~~~~~~~~~~~~~~~~~~~~~

When skims are developed, it is important to test the performance of the skim on a data and on a range of background MC samples. Two command-line tools are provided in the skim package to aid in this: :ref:`b2skim-stats-submit<b2skim-stats-submit>` and :ref:`b2skim-stats-print<b2skim-stats-print>`. They are available in the ``PATH`` after setting up the ``basf2`` environment after calling ``b2setup``. The former submits a series of test jobs for a skim on data and MC samples, and the latter uses the output files of the jobs to calculate performance statistics for each sample including retention rate, CPU time, and uDST size per event. ``b2skim-stats-print`` also provides estimates for combined MC samples, where the statistics are weighted by the cross-section of each background process.

First run ``b2skim-stats-submit``, which will submit small skim jobs on test files of MC and data using ``bsub``. For example,

.. code-block:: sh

    b2skim-stats-submit -s LeptonicUntagged SLUntagged

Monitor your jobs with ``bjobs -u USERNAME``. Once all of the submitted jobs have completed successfully, then run ``b2skim-stats-print``.

.. code-block:: sh

    b2skim-stats-print -s LeptonicUntagged SLUntagged

This will read the output files of the test jobs, and produce tables of statistics in a variety of outputs.

* A subset of the statistics printed to the screen per-skim. This output is for display only.

* If the ``-M`` flag is provided, a Markdown table will be written to ``skimStats.md``. This table is in a format that can be copied into the comment fields of pull requests (where BitBucket will format the table nicely for you). Use this flag when asked to produce a table of stats in a pull request.

* If the ``-J`` flag is provided, a text file ``confluenceTables.txt`` is written, in which the statistics are formatted as Confluence wiki markup tables. These tables can be copied directly onto a Confluence page by editing the page, selecting ``Insert more content`` from the toolbar, selecting ``Markup`` from the drop-down menu, and then pasting the content of the text file into the markup editor which appears. Confluence will then format the tables and headings. The markup editor can also be accessed via ``ctrl-shift-D`` (``cmd-shift-D``).

* If the ``-J`` flag is provided, then all statistics produced are printed to a JSON file ``skimStats.json``, indexed by skim, statistic, and sample label. This file is to be used by grid production tools.

.. tip::
   To test your own newly-developed skim, make sure you have followed all the instructions in
   `writing-skims`, particularly the instructions regarding the skim registry.


.. _b2skim-stats-submit:

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


.. _b2skim-stats-print:

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

      This will be included in a future version of the externals.


.. _b2skim-prod:

``b2skim-prod-json``: Produce grid production requests
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. argparse::
   :filename: skim/tools/b2skim-prod-json
   :func: get_argument_parser
   :prog: b2skim-prod-json
   :nodefaultconst:
   :nogroupsections:

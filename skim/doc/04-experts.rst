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
       If your skim does not define ``__description__``, ``__category__``, ``__authors__``, ``__contact__``, or ``build_lists``, then you will see an error message like:

       ::

           TypeError: Can't instantiate abstract class SinglePhotonDark with abstract methods __authors__

       This can be fixed by defining these required attributes and methods.

3. If you require any standard lists to be loaded for your skim, override the method ``load_standard_lists``. This will be run before ``build_lists`` and ``additional_setup``.

   This step is separated into its own function so that the `CombinedSkim` class can do special handling of these functions to avoid accidentally loading a standard list twice when combinining skims.

4. If any further setup is required, then override the ``additional_setup`` method.

5. *[Mandatory]* Define all cuts by overriding ``build_lists``. Before the end of the ``build_lists`` method, the attribute ``SkimLists`` must be set to a list of skim list names.

6. Skims can crash on the grid if the log files are too large. If any modules is producing too much output, then override the attribute ``NoisyModules`` as a list of such modules, and their output will be set to print only error-level messages.

7. By default, the skim test file is a neutral :math:`B` pair sample with beam background. If your skim has a retention rate of close to zero for this sample type, you may wish to override the attribute ``TestFiles``. This should be a list of file names retrieved from `skimExpertFunctions.get_test_file`, such as:

   .. code-block:: python

       TestFiles = [get_test_file("MC13_ggBGx1")]

8. *[Mandatory]* Add your skim to the registry, with an appropriate skim code (see :ref:`Skim Registry<skim-registry>`).

With all of these steps followed, you will now be able to run your skim using the :ref:`skim command line tools<skim-running>`. To make sure that you skim does what you expect, and is feasible to put into production, please also complete the following steps:

9. Test your skim! The primary point of skims is to be run on the grid, so you want to be sure that the retention rate and processing time are low enough to make this process smooth.

   The skim package contains a set of tools to make this straightforward for you. See `Testing skim performance`_ for more details.

10. Define validation histograms for your skim by overriding the method `BaseSkim.validation_histograms`, and running `b2skim-generate-validation<b2skim-generate-validation>` to auto-generate a steering file in the skim validation directory. The :py:func:`validation_histograms <skimExpertFunctions.BaseSkim.validation_histograms>` method should not be long: it should simply use the particle lists that have been created by :py:func:`build_lists <skimExpertFunctions.BaseSkim.build_lists>` to plot one or two key variables. If possible, *do not do any further reconstruction or particle list loading here*. Below is an example of what a typical method ought to contain.

    .. code-block:: python

        def validation_histograms(self, path):
            # The validation package is not part of the light releases, so this import
            # must be made inside this function rather than at the top of the file.
            from validation_tools.metadata import create_validation_histograms

            # Combine B+ particle lists for a single histogram (assuming self.SkimLists only
            # has B+ particle lists). Not necessary if only one particle list is created.
            ma.copyLists(f"B+:{self}_validation", self.SkimLists, path=path)
        
            create_validation_histograms(
                rootfile=f"{self}_validation.root",
                particlelist=f"B+:{self}_validation",
                variables_1d=[
                    ("deltaE", 20, -0.5, 0.5, "#Delta E", __liaison__,
                     "$\\Delta E$ distribution of reconstructed $B^{+}$ candidates",
                     "Peak around 0", "#Delta E [GeV]", "B^{+} candidates"),
                    # Include "shifter" flag to have this plot shown to shifters
                    ("Mbc", 20, 5.2, 5.3, "M_{bc}", __liaison__,
                     "$M_{\\rm bc}$ distribution of reconstructed $B^{+}$ candidates",
                     "Peak around 5.28", "M_{bc} [GeV]", "B^{+} candidates", "shifter")],
            )

    .. seealso::

       Documentation of :py:func:`create_validation_histograms <validation_tools.metadata.create_validation_histograms>` for explanation of the expected arguments. Options to pay particular attention to:

       * Passing the "shifter" flag in ``metaoptions``, which will allow the plot to be shown to shifters when they check `validation.belle2.org <https://validation.belle2.org>`_.

       * Adding a contact email address with the ``contact`` option, preferably the contact email of your working group's skim liaison. If this is set, then the B2Bot will know where to send polite emails in case the validation comparison fails.

.. _skim-steering-file:

Building skim lists in a steering file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Calling an instance of a skim class will run the particle list loaders, setup function, list builder function, and uDST output function. So a minimal skim steering file might consist of the following:

.. code-block:: python

    import basf2 as b2
    import modularAnalysis as ma
    from skim.foo import MySkim

    path = b2.Path()
    ma.inputMdstList("default", [], path=path)
    skim = MySkim()
    skim(path)  # __call__ method loads standard lists, creates skim lists, and saves to uDST
    b2.process(path)

After ``skim(path)`` has been called, the skim list names are stored in the Python list ``skim.SkimLists``.

.. warning:: There is a subtle but important technicality here: if ``BaseSkim.skim_event_cuts`` has been called, then the skim lists are not built for all events on the path, but they are built for all events on a conditional path. A side-effect of this is that no post-skim path can be safely defined for the `CombinedSkim` class (since a combined skim of five skims may have up to five conditional paths).

After a skim has been added to the path, the attribute `BaseSkim.postskim_path` contains a safe path for adding subsequent modules to (*e.g.* performing further reconstruction using the skim candidates). However, the final call to `basf2.process` must be passed the original (main) path.

.. code-block:: python

    skim = MySkim()
    skim(path)
    # Add subsequent modules to skim.postskim_path
    ma.variablesToNtuple(skim.SkimLists[0], ["pt", "E"], path=skim.postskim_path)
    # Process full path
    b2.process(path)

The above code snippet will produce both uDST and ntuple output. To only build the skim lists without writing to uDST, pass the configuration parameter ``outputUdst=False`` during initialisation of the skim object:

.. code-block:: python

    skim = MySkim(udstOutput=False)
    skim(path)

Disabling uDST output may be useful to you if you want to do any of the following:

* print the statistics of the skim without producing any output files,

* build the skim lists and perform further reconstruction or fitting on the skim candidates before writing the ROOT output,

* go directly from unskimmed MDST to analysis ntuples in a single steering file (but please consider first using the centrally-produce skimmed uDSTs), or

* use the :ref:`skim flag<skim-flags>` to build the skim lists and write an event-level ntuple with information about which events pass the skim.

.. tip::

    The tool :ref:`b2skim-generate<b2skim-generate>` can be used to generate simple skim steering files like the example above. The tool :ref:`b2skim-run<b2skim-run>` is a standalone tool for running skims. :ref:`b2skim-run<b2skim-run>` is preferable for quickly testing a skim during skim development. :ref:`b2skim-generate<b2skim-generate>` should be used as a starting point if you are doing anything more complicated than simply running a skim on an MDST file to produce a uDST file.


.. _skim-flags:

Skim flags
..........

When a skim is added to the path, an event-level variable is created (via an alias), which indicates whether an event passes the skim or not. It is of the form ``passes_<SKIMNAME>``, and can be accessed through the property ``BaseSkim.flag``.

The same caveat from the previous section regarding ``postskim_path`` applies here. The skim flag is not guaranteed to work if used on the main path, because the skim lists may not be built for all events.

In the below code snippet, we build the skim lists, skip the uDST output, and write an ntuple containing the skim flag and other event-level variables:

.. code-block:: python

    skim = MySkim(udstOutput=False)
    skim(path)
    # Add subsequent modules to skim.postskim_path, including anything that uses skim.flag
    ma.variablesToNtuple("", [skim.flag, "nTracks"], path=skim.postskim_path)
    # Process full path
    b2.process(path)


.. _skim-running:

Running a skim
~~~~~~~~~~~~~~

In the skim package, there are command-line tools available for running skims, documented below. These take a skim name as a command line argument, and run the code defined in the corresponding subclass of `BaseSkim`.

.. _b2skim-run:

``b2skim-run``: Run a skim
..........................

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

.. _b2skim-generate:

``b2skim-generate``: Generate skim steering files
.................................................

.. tip::
   This tool is for cases where other tools does not suffice (such as running on the grid, or adding
   additional modules to the path after adding a skim.). If you just want to run a skim on KEKCC,
   consider using :ref:`b2skim-run<b2skim-run>`. If you want to test the performance of your skim,
   consider using the :ref:`b2skim-stats tools<testing-skims>`.

.. argparse::
   :filename: skim/tools/b2skim-generate
   :func: get_argument_parser
   :prog: b2skim-generate
   :nodefaultconst:
   :nogroupsections:

.. _b2skim-generate-validation:

``b2skim-generate-validation``: Generate skim validation scripts
................................................................

.. argparse::
   :filename: skim/tools/b2skim-generate-validation
   :func: get_argument_parser
   :prog: b2skim-generate-validation
   :nodefaultconst:
   :nogroupsections:


Skim tutorial
~~~~~~~~~~~~~

A Jupyter notebook skimming tutorial can be found in ``skim/tutorial/Skimming_Tutorial.ipynb`` in basf2.

.. _skim-registry:

Skim registry
~~~~~~~~~~~~~

All skims must be registered and encoded by the relevant skim liaison. Registering a skim is as simple as adding it to the list in ``skim/scripts/skim/registry.py`` as an entry of the form ``(SkimCode, ParentModule, SkimName)``.

The skim numbering convention is defined on the `Confluence skim page`_.

.. _Confluence skim page: https://confluence.desy.de/display/BI/Skimming+Homepage#SkimmingHomepage-Skimcodeconventionandskimregistry

.. automodule:: skim.registry
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

* By default, a subset of the statistics are printed to the screen.

* If the ``-M`` flag is provided, a Markdown table will be written to ``SkimStats.md``. This table is in a format that can be copied into the comment fields of pull requests (where BitBucket will format the table nicely for you). Use this flag when asked to produce a table of stats in a pull request.

* If the ``-C`` flag is provided, a text file ``SkimStats.txt`` is written, in which the statistics are formatted as Confluence wiki markup tables. These tables can be copied directly onto a Confluence page by editing the page, selecting ``Insert more content`` from the toolbar, selecting ``Markup`` from the drop-down menu, and then pasting the content of the text file into the markup editor which appears. Confluence will then format the tables and headings. The markup editor can also be accessed via ``ctrl-shift-D`` (``cmd-shift-D``).

* If the ``-J`` flag is provided, then all statistics produced are printed to a JSON file ``SkimStats.json``, indexed by skim, statistic, and sample label. This file contains extra metadata about when and how the tests were run. This file is to be used by grid production tools.

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


.. _skim-expert-functions:

Utility functions for skim experts
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The module ``skimExpertFunctions`` contains helper functions to perform common tasks relating to skims. Importantly, it contains the `skimExpertFunctions.BaseSkim` class, which is how skims are defined.

.. automodule:: skimExpertFunctions
    :members:
    :undoc-members:


.. _b2skim-prod:

``b2skim-prod``: Produce grid production requests
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. argparse::
   :filename: skim/tools/b2skim-prod
   :func: get_argument_parser
   :prog: b2skim-prod
   :nodefaultconst:
   :nogroupsections:


.. _b2skim-stats-total:

``b2skim-stats-total``: Produce summary statistics for skim package
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. argparse::
   :filename: skim/tools/b2skim-stats-total
   :func: get_argument_parser
   :prog: b2skim-stats-total
   :nodefaultconst:
   :nogroupsections:


.. _lpns2yaml:

``lpns2yaml.py``: Convert lists of LPNs to format expected by ``b2skim-prod``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. argparse::
   :filename: skim/tools/lpns2yaml.py
   :func: get_argument_parser
   :prog: lpns2yaml.py
   :nodefaultconst:
   :nogroupsections:

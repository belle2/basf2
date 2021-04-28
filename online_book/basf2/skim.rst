.. _onlinebook_skim:

Skimming
========

.. sidebar:: Overview
    :class: overview

    **Teaching**: 25 min

    **Exercises**: 25 min

    **Prerequisites**:

        * :ref:`gbasf2 lesson <onlinebook_gbasf2>`

    **Questions**:

        * What are skims, and why should analysts use skims?

        * How can I find information about what skims are available?

    **Objectives**:

        * Find documentation about available skims.

        * Run a skim on a file using the command-line tool :ref:`b2skim-run
          <b2skim-run>`.

        * Add an existing skim to a steering file.

        * Find information about skims available on the grid.


What is skimming?
-----------------

Skims are sets of selections made on data and MC with particular analyses in
mind. The purpose of skims is to produce data and MC files that have been
reduced from their original size. This is done by applying a list of criteria to
the data and MC, such that only events that interested a given analyst will be
stored and provided. The analyst can then use the skimmed samples to further
fine tune and improve their research. Skimmed samples are usually around 90%
smaller than the original data and MC samples they are produced from. These
samples are thus more manageable to use for analysis development and reduce the
overall CPU and storage usage requirements of each analyst. Belle II is
expecting to collect 50 ab-1 of data, which will be almost impossible to run on
without skimming.

The criteria for skims varies from analysis to analysis. The general gist is to
use a loose selection which can then be optimized by the analyst. For example,
an analyst looking for the decay of a :math:`B\to D \ell \nu` and :math:`D^0 \to
K^- \pi^+` will want to examine events where there are at least 3 tracks: two
for the :math:`D` daughter tracks and one for the lepton. The corresponding skim
can include such a criteria where only events with more 3 tracks or more are
included. The skim will also include a loose selection for the reconstruction of
a :math:`B` meson. Tighter selection criteria related to the lepton or D
reconstruction are usually not applied at skim level. The analyst applied their
optimized selection on the skimmed samples.

Skims are intended into reduce the overall CPU requirements of the
collaboration, and to make your life easier. Skims can make your life easier in
the following ways:

    * Skimmed files are generally less than 10% the size of the original
      (unskimmed) files, so your steering file will not need to process as many
      events, and your jobs will finish quicker.

    * The particles reconstructed during a skim are available when you load in
      the skimmed uDST, so you can use these in further reconstruction. For
      example, there are skims which use the :ref:`FEI <onlinebook_fei>`, so
      this computationally expensive reconstruction is performed during the
      skimming step and does not need to be repeated in later reconstruction.


Mechanics of a skim
~~~~~~~~~~~~~~~~~~~

Under the hood, skims operate by running over the mDST, which is the format
of the produced Monte Carlo samples and the processed data at Belle II,
reconstructing a specific particle list, and writing the reconstructed particle
information to a microDST, referred to as uDST. The skim filter removes any
events which do not satisfy the criteria of the given skim, and thus do not have
any candidates in the skim particle lists. For example, for the decay of
:math:`B\to D \ell \nu` and :math:`D^0 \to K^- \pi^+`, all events with less than
3 tracks are not included. Furthermore, in the skim itself, :math:`B` meson is
reconstructed using very loose criteria on the lepton and :math:`D` daughters.
Events that do not have a :math:`B` candidate satisfying the loose criteria
defined by the skim will not be included.

.. admonition:: Question
     :class: exercise stacked

     uDST files contain the skimmed particle list information in addition to all
     the information contained in the mDST. Why is the file size of a uDST skim
     still smaller than the original size of the mDST?

.. admonition:: Solution
     :class: toggle solution

     Even though we are adding more information to each event by saving the
     reconstructed particle lists, only a fraction of events are kept by the
     skim, so the overall file sizes are reduced.


List of available skims
-----------------------

At Belle II, we already have a list of skims developed by different analysts and
skim liaisons.
All available skims are listed on :ref:`Sphinx <skim_physics>` (although not all
of these are produced in skim campaigns). Although we try to keep the docstrings
for each skim up-to-date, the best way to find out what selections are in a skim
is to read the source code. The most important part of a skim's source code is
the ``build_lists`` method, where particles are reconstructed and selections are
applied.

.. admonition:: Exercise
     :class: exercise stacked

     Find the source code for the electroweak penguin (EWP) skims by navigating
     the Sphinx documentation.

.. admonition:: Solution
     :class: toggle solution

     Click the ``[Source]`` button on any of the skims in the :ref:`EWP
     <skim_physics_ewp>` section to be taken to source code for that skim.


Running a skim locally
----------------------

As mentioned above, there is a list of developed skims available in the skim package.
An analyst starting a new project is strongly encouraged to browse through the list
of available skims and find out if there is a skim that meets their needs.
Available skims are ready to run on any data and MC sample.

There are two ways to run a skim yourself: including the skim in a steering
file, or using the command-line tool ``b2skim-run``.

Including a skim in a steering file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Skims in the skim package are defined via the ``BaseSkim`` class. To add all the
required modules for a skim to your steering file, simply run:

.. code-block:: python

    from skim.leptonic import LeptonicUntagged
    skim = LeptonicUntagged()
    skim(path)  # add required skim modules to path

Running the above code will add modules to the path to load so-called *standard
particle lists*, reconstruct the skim particle lists, and write the particle
list to an output uDST file. If you would like to disable the uDST output, you
can do so via:

.. code-block:: python

    skim(path, udstOutput=False)

Once the skim modules have been added to the path, you can retrieve a Python
list of particle lists:

.. code-block:: python

    >>> skim.SkimLists
    ["B-:LeptonicUntagged_0", "B-:LeptonicUntagged_1"]

You can then use this list of particle list names in further reconstruction or
ntuple output.


Using ``b2skim-run``
~~~~~~~~~~~~~~~~~~~~

The command ``b2skim-run`` is a simple tool for applying a skim to a sample.

.. code-block:: bash

    b2skim-run single SkimName -i MyDataFilename.mdst.root

By default the output filename will simply be the corresponding skim code (more
on this in the next part of the lesson), but this can be controlled with the
``-o`` flag.

The full documentation of this tool can be found :ref:`on Sphinx <b2skim-run>`,
or by using the ``-h`` flag.

.. admonition:: Exercise
     :class: exercise stacked

     Use ``b2skim-run`` to apply the skim ``XToD0_D0ToHpJm`` to the file
     ``$BELLE2_VALIDATION_DATA_DIR/mdst13.root``.

.. admonition:: Solution
     :class: toggle solution

     The command to run the ``XToD0_D0ToHpJm`` skim on this sample is:

     .. code-block:: bash

         b2skim-run single XToD0_D0ToHpJm -i $BELLE2_VALIDATION_DATA_DIR/mdst13.root

     By default, this will output a uDST file in the current directory titled
     ``17230100.udst.root``.

.. admonition:: Exercise
     :class: exercise stacked

     What is the retention rate (fraction of events passing the skim) of the
     ``XToD0_D0ToHpJm`` skim on this sample?

.. admonition:: Hint
     :class: toggle xhint stacked

     You can use the tool ``b2file-metadata-show`` to print the number of events
     in an mDST or uDST file.

.. admonition:: Solution
     :class: toggle solution

     .. code-block:: bash

         b2file-metadata-show $BELLE2_VALIDATION_DATA_DIR/mdst13.root
         b2file-metadata-show 17230100.udst.root

     We find the unskimmed file has 10124 events, and the skimmed file has 228
     events, so the retention rate on this sample is 2.2%.


Accessing skims on the grid
---------------------------

Analysts do not have to run the skims themselves on data or generic MC.
Each new MC campaign or data collection, a list of skims is requested by the
analysts in the Belle II physics working groups. This is done via the skim
liaison or via JIRA tickets. Once requested, the skim is run on the large MC
and/or data samples by the skim production managers. These skims are then
announced when ready and made available to the analyst.


Each skim campaign on data or MC samples  has a given name. For example, skims of MC13a
run-independent MC are listed under the campaign name ``SkimM13ax1``. Skims of data are usually
made available for official processing, like ``Proc11``, or for individual buckets like ``bucket9``,
``bucket10``, etc..The corresponding skim campaign names are ``SkimP11x1`` and ``SkimB9x1-SkimB13x1``.
The production status of available MC and data samples is continuously updated on the
`Data Production Status <https://confluence.desy.de/display/BI/Data+Production+Status>`_ page.
Status updates on the readiness of a skim campaign are also posted on the `Skim Confluence page
<https://confluence.desy.de/pages/viewpage.action?pageId=167963852>`_.
For example, you can browse `here <https://confluence.desy.de/pages/viewpage.action?pageId=167963852>`_
for the latest updates on 2020a,b data skims.

To find the list of skim campaign campaigns available on the, simply browse through the app,
select Data type: MC or Data and look in the drop-down menu under Campaigns.  All skim campaigns
start with the not so mysterious name "Skim".


Skimmed samples are produced and stored on the grid. The output LFNs are
documented on the *dataset searcher*. You can then run your analysis these
centrally-produced skims with :ref:`gbasf2 <onlinebook_gbasf2>`.
LFNs on the grid have a maximum length restriction, so we can't include the
plain skim name in the LFN. Instead, we have standardised eight-digit *skim
codes* to identify skims. When searching for skimmed datasets on the grid, use
the skim codes. The documentation of each skim on :ref:`Sphinx <skim_physics>`
contains its corresponding skim code, and the full table of codes can be found
in the documentation of `skim.registry.SkimRegistryClass`.


.. note::

    The details of the numbering scheme are explained on `the skimming
    Confluence page
    <https://confluence.desy.de/x/qw36Ag#SkimmingHomepage-Skimcodeconventionandskimregistry>`_.


.. admonition:: Exercise
     :class: exercise stacked

     Use the dataset searcher to get the list of LPNs for the ``B0toDpi_Kspi``
     skim from the MC skim campaign ``SkimM13ax1``.

.. admonition:: Hint
     :class: toggle xhint stacked

     Find the skim code for ``B0toDpi_Kspi`` on the :ref:`skim documentation on
     Sphinx <skim_physics>`.

.. admonition:: Solution
     :class: toggle solution

     Visit the DIRAC webapp and navigate to the dataset searcher. The LFNs can
     be found by selecting ``MC`` and ``BGx1``, and passing ``SkimM13ax1`` in the
     ``Campaigns`` field, and ``14120601`` in the ``Skim Types`` field.


.. tip::

    All skims on the grid are produced using some release of the software. If
    you're unsure which version was used to produce your skim, check the LFN, as
    it is recorded in there! You can then directly read source code for that
    release to find the skim definitions.


.. admonition:: Exercise
     :class: exercise

     Run the analysis script in `B2A303-MultipleDecays-Reconstruction.py
     <https://stash.desy.de/projects/B2/repos/software/browse/analysis/examples/tutorials/B2A303-MultipleDecays-Reconstruction.py>`_
     on one of the LPNs for the mixed samples of the ``B0toDpi_Kspi``
     skim from the MC skim campaign ``SkimM13ax1`` on the grid.


Skimmed data samples are made available in directories on the grid, where each
directory corresponds to a given run. This results in an inconvenient number of
directories the user has to run on, however, this preserves the run information of
a given skim, as inherited from data production.

.. warning::

    Currently the dataset searcher
    does not list all available directories for a given skim production job. It only
    lists one directory. In reality, there are usually ~100 directories per production.
    This is a known bug and will be improved in future developments of the dataset searcher.

For now, a workaround in order is described on the
`Skim Confluence page <https://confluence.desy.de/pages/viewpage.action?pageId=167963852>`_
to run your analysis script on the full set
of skimmed data samples available for a given campaign.



Getting involved
----------------

Each working group has an assigned skim liaison (all `listed on Confluence
<https://confluence.desy.de/x/qw36Ag#SkimmingHomepage-Skimmingpersonnel>`_),
whose job it is to survey the needs of the group and develop skims. If there is
an existing skim that might be useful for your analysis and is not currently
being produced, talk to your local skim liaison.

If you would like to get more involved in the writing and testing of skims, then
you may find the :ref:`skim experts section <writing-skims>` of the Sphinx
documentation helpful.


.. admonition:: Key points
    :class: key-points

    * The two sources of documentation on skims are the :ref:`Sphinx
      documentation <skim>` and the `skimming Confluence page
      <https://confluence.desy.de/x/qw36Ag>`_. The best way to find out how a
      particular skim is currently defined is to read the source code (either on
      Sphinx, or in the directory ``skim/scripts/skim/`` in the software repo).

    * You can run a skim by adding a short segment of code to your steering
      file, or by using the command-line tool ``b2skim-run``.

    * Centrally-produced skims can be accessed on the grid with gbasf2. Use the
      dataset searcher to locate skimmed data by using the relevant skim code.

    * Running on skimmed data and MC can make your life as an analyst easier.
      However, skims are only useful if they are developed through communication
      between analysts and skim liaisons, so don't hesitate to contact your
      working group's liaison.

.. include:: ../lesson_footer.rstinclude

.. topic:: Authors of this lesson

     Phil Grace, Racha Cheaib


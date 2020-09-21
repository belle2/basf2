.. _onlinebook_skim:

Skimming
========

.. include:: ../todo.rst

.. sidebar:: Overview
    :class: overview

    **Teaching**: ?? min

    **Exercises**: ?? min

    **Prerequisites**: 
    	
    	* 

    	* :ref:`gbasf2 <onlinebook_gbasf2>`

    **Questions**:

        * Why should analysts use skims?

    **Objectives**:

        * Find documentation about available skims.

        * Run a skim on a file using the command-line tool :ref:`b2skim-run
          <b2skim-run>`.

        * Add an existing skim to a steering file.

        * Find information about skims available on the grid.


.. admonition:: Key points
    :class: key-points

    * The two sources of documentation on skims are the :ref:`Sphinx
      documentation <skim>` and the `skimming Confluence page
      <https://confluence.desy.de/display/BI/Skimming+Homepage>`_.

    * 


What is skimming?
-----------------

Skims are sets of selections made on data and MC with particular analyses in
mind. The selections deliberately chosen to be relatively loose, so that the
same selections can be used and refined by multiple analysts.

For example, a simple skim targeting :math:`B\to D` analyses might reconstruct a
:math:`D` meson with loose selections via several modes, and apply
continuum-suppression cuts to remove events without a pair of :math:`B` mesons.

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
      skimming step and does not need to be repeated in further reconstruction.


Mechanics of a skim
~~~~~~~~~~~~~~~~~~~

Under the hood, skims operate by reconstructing some set of particles, and
writing the reconstructed particle information to a uDST. The skim filter
removes any events which do not have any particles in the skim particle lists.

.. admonition:: Question
     :class: exercise stacked

     What is the difference between the MDST and uDST formats?

.. admonition:: Solution
     :class: toggle solution

     uDST files contain particle information, while MDST files do not.

     .. note::

         Even though we are adding more information to the files during
         skimming, the lower number of output events means the file sizes are
         still reduced.


List of available skims
-----------------------

All available skims are listed on :ref:`Sphinx <skim_physics>` (although not all
of these are produced in skim campaigns). Although we try to keep the docstrings
for each skim up-to-date, the best way to find out what selections are in a skim
are to read the source code. The most important part of a skim's source code is
the ``build_lists`` method, where the particle reconstruction and selections are
done.

.. admonition:: Question
     :class: exercise stacked

     Find the source code for the electroweak penguin (EWP) skims by navigating
     the Sphinx documentation.

.. admonition:: Solution
     :class: toggle solution

     Click the ``[Source]`` button on any of the skims in the :ref:`EWP
     <skim_physics_ewp>` section to be taken to source code for that skim.


Running a skim locally
----------------------

.. note::

   Things to be explained here:

       * Exercise of adding a skim to a steering file.

       * Run a skim via ``b2skim-run single <SkimName> -i <in> -o <out>``


Accessing skims on the grid
---------------------------

If a skim is requested by a working group, then it is applied to datasets on the
grid during a skimming campaign and the output LFNs are documented on the
*dataset searcher*. You can access these centrally-produced skims with
:ref:`gbasf2 <onlinebook_gbasf2>`.

LFNs on the grid have a maximum length restriction, so we can't include the
plain skim name in the LFN. Instead, we have standardised eight-digit *skim
codes* to identify skims. When searching for skimmed datasets on the grid, use
the skim codes. The documentation of each skim on :ref:`Sphinx <skim_physics>`
contains its corresponding skim code.


.. note::

    The details of the numbering scheme are explained on `the skimming
    Confluence page
    <https://confluence.desy.de/display/BI/Skimming+Homepage#SkimmingHomepage-Skimcodeconventionandskimregistry>`_.


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


Getting involved
----------------

Each working group has an assigned skim liaison, whose job it is to survey the
needs of the group and develop skims (all `listed on Confluence
<https://confluence.desy.de/display/BI/Skimming+Homepage#SkimmingHomepage-Skimmingpersonnel>`_).
If there is an existing skim that might be useful for your analysis and is not
currently being produced, talk to you local skim liaison.

If you would like to get more involved in the writing and testing of skims, then
you may find the :ref:`skim experts section <writing-skims>` of the
Sphinx documentation helpful.


.. topic:: Author of this lesson

     Phil Grace


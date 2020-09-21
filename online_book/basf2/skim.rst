.. _onlinebook_skim:

Skimming
========

.. include:: ../todo.rst

.. sidebar:: Overview
    :class: overview

    **Teaching**: 

    **Exercises**: 

    **Prerequisites**: 
    	
    	* 

    **Questions**:

        * Why should analysts use skims?

    **Objectives**:

        * Find documentation about available skims.

        * Add an existing skim to a steering file.

        * Run a skim on a file using the command-line tool :ref:`b2skim-run <b2skim-run>`.

        * Find information about skims available on the grid.


.. admonition:: Key points
    :class: key-points

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
-------------------

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

:ref:`list of skims on Sphinx <skim_physics>`

Read the source code (in particular, the ``build_lists`` method of each skim). ``skim/scripts/skim/``

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




Accessing skims on the grid
---------------------------

If a skim is requested by a working group, then it is produced on the grid 

:ref:`gbasf2 <onlinebook_gbasf2>`


Skims and you
-------------

Each working group has an assigned skim liaison, whose job it is to survey the
needs of the group and develop skims.



.. topic:: Author of this lesson

     Phil Grace

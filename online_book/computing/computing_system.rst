.. _onlinebook_computing_system:

The computing system
====================

.. sidebar:: Overview
    :class: overview

    **Teaching**: 20 min

    **Exercises**: 10-20 min

    **Prerequisites**:

        * An internet browser
        * DESY accounts
        * :ref:`onlinebook_collaborative_tools` lesson.
        * An ``ssh`` client and the :ref:`onlinebook_ssh` lesson.
        * Basic bash

    **Questions**:

        * What is the grid
        * What is a basf2 module, path, steering file?
        *

    **Objectives**:

        * Understand
        * Find the right chapters in this documentation.
        * Run basf2 and display a list of variables
        * Run basf2 in interactive ipython shell mode.
        * Access the help in 3 different ways.
        * Confidently read the source code for a `modularAnalysis` convenience
          function.


The distributed computing system (a.k.a. "The Grid")
----------------------------------------------------

To achieve the physics goals of the experiment, the data has to be reprocessed, distributed along the collaborators
and analyzed.
The large luminosity to be delivered by the Belle II experiment projects that we will handle tens of Peta bytes
per year. It is hard to expect that a single site provides the computing resources to manage such large data set.
Additionally, Belle II is a worldwide collaboration with more than 1000 scientists working in different regions
of the planet.
Therefore it is natural to adopt a distributed computing architecture in order to access data and obtain physics results
in a feasible time.

The distributed computing system, also known as the grid, is a form of computing where a "virtual super computer" is
composed of many networked loosely computers.
It manages the processing of the raw data, skimming and analysis, as well as the production of MC samples.

.. figure:: B2computingModel.png
    :align: center
    :width: 400px
    :alt: The Belle II grid

    Snapshot of the Belle II grid, composed by 60 computing sites around the world.

In our computing model, all raw data produced by the experiment is uploaded and registered on the grid. After the
calibration is performed, data is reprocessed in the raw data centers to produce mdst and (skimmed) udst files.
In parallel, MC samples are centrally produced and stored. Details about the produced samples are usually available at
the `Data Production Confluence pages <https://confluence.desy.de/display/BI/Data+production+WebHome>`_.

Access to the data available on the grid is performed

.. figure:: B2computingModel.png
    :align: center
    :width: 400px
    :alt: Computing model

    The Belle II distributed computing model.

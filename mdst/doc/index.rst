.. _mdst:

====================
Belle II File Format
====================

The official standard Belle II file format is the mini data-summary table (mdst).

There is a basf2 package (the mdst package) which defines this file format.
An mdst file contains a curated list of **post-reconstruction dataobjects** which are provided for analysis use.
The dataobjects are particularly important as they are the (only) information that is provided for high-level analysis.

These *mdst dataobjects* are optimised for minimal disk size per event.
This is important because the Belle II experiment will collect around :math:`5\times10^{10}` events, so data file size considerations will become rather critical.

.. seealso:: 

    `What is the difference between an mdst and a dst?
    <https://questions.belle2.org/question/219/what-is-the-difference-between-an-mdst-and-a-dst/>`_

Python interface
================

.. automodule:: mdst
   :members:

C++ dataobjects
===============

The post-reconstruction dataobjects are C++ classes found in the mdst dataobjects directory::

        ls $BELLE2_RELASE_DIR/mdst/dataobjects/include

.. TODO can I link to the actual directory on sphinx here? I would obviously need to link to the release-tag that we're looking at right now.

Related packages
================

The :ref:`skim` package defines a set of selection criteria to be applied to mdst files.
This will result in :ref:`analysis_udstoutput` (udst) files which also contain particle candidates.

.. seealso:: :ref:`analysis_particles`

.. seealso:: :ref:`analysis_udstoutput`

.. seealso:: :ref:`skim`

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

Or in the C++ doxygen documentation `here <https://software.belle2.org/|release|/group__mdst__dataobjects.html>`__.

ROOT and compatibility guarantee
================================

Mdst files are written by the :b2:mod:`RootOutput` module and are based on the ROOT file format.
However it is important to note that analysis of mdst with any software other than basf2 is not supported or permitted.
I.e. use of the basf2 framework and the :ref:`analysis` package is mandatory.

.. warning:: 

        A common misconception: Opening an mdst file with standard ROOT tools (e.g. with a TBrowser) may initially "work", 
        but the results are not reproducible.

        Many dataobject member accessors require the basf2 environment to return meaningful values.
        Segmentation faults and double-counting of tracks and/or clusters are very likely to be encountered.

However! Backward-compatibility is guaranteed for two major basf2 releases and the supported light releases.

.. seealso:: `Backward compatibility <https://confluence.desy.de/display/BI/Backward+Compatibility>`__ on confluence.

Related packages
================

The :ref:`skim` package defines a set of selection criteria to be applied to mdst files.
This will result in :ref:`analysis_udstoutput` (udst) files which also contain particle candidates.

.. seealso:: :ref:`analysis_particles`

.. seealso:: :ref:`analysis_udstoutput`

.. seealso:: :ref:`skim`

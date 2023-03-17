
===========
Calibration
===========

.. _calibration_caf:

Calibration processes at Belle II are designed to be run within the basf2 environment so that any
basf2 user can develop and run them without needing to install anything else.
The calibration package contains three main components:

#. The calibration framework C++ code that defines base classes and basf2 modules to extract calibration
   data objects during a basf2 process and save them.

#. The calibration framework Python code which is essentially a job submission and monitoring system.

#. The prompt calibration scripts which define the processes that the automated calibration system will use.

This document focuses on the Python CAF code and the vaious tools provided to run prompt calibrations. 

Calibration Framework (CAF)
===========================

.. toctree::
    :maxdepth: 2
    :glob:

    caf

Prompt Calibration
==================

.. toctree::
    :maxdepth: 2
    :glob:

    prompt

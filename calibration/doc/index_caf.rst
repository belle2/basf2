:tocdepth: 3

===========
Calibration
===========

The Calibration Framework (CAF)
===============================

The Python CAF is a set of Python modules and classes which work together to provide users with a
convienient interface to running calibration jobs. It is assumed that you are using the C++
calibration framework classes e.g. the CalibrationCollectorModule and CalibrationAlgorithm
class in the calibration package. These provide a consistent interface for running calibration
jobs on input data which the Python CAF can automate for you.

Essentially you can think of the Python CAF as providing a way to automatically create basf2 scripts
that run your Collector module and Algorithm for you. That way you don't have to create multiple
shell submission scripts and any improvements made to the CAF will benefit you. It is important to
remember that when you are running a CAF process, you are **not** running a `basf2` process. You are
running a Python script that will create basf2 processes for you.


The Calibration Class
---------------------

The first part of the CAF to understand is the :py:class:`Calibration <caf.framework.Calibration>` class.
This is where you specify which Collector module and which Algorithms you want to run.
It is also where you can configure the basf2 processes before they are run for you.
This includes setting the input data files, different global tags, and making reconstruction paths to run
prior to your Collector module.


.. autoclass:: caf.framework.Calibration
   :members:
   :show-inheritance:

The CAF Class
-------------

.. autoclass:: caf.framework.CAF
   :members:

Job Submission Backends
-----------------------

.. automodule:: caf.backends
   :members:
   :inherited-members:
   :show-inheritance:

Utility Functions + Objects
---------------------------

.. automodule:: caf.utils
   :members:

Advanced Usage
--------------

There are several options for the CAF that most users will never need.
But for more adventurous calibration developers there are quite a lot of configuration options and possibilities for extending
the functionality of the CAF.

Writing Your Own Calibration Class
++++++++++++++++++++++++++++++++++

.. autoclass:: caf.framework.CalibrationBase
   :members:
   :inherited-members:
   :show-inheritance:

Strategy Objects
++++++++++++++++

.. autoclass:: caf.framework.Algorithm
   :members:

.. automodule:: caf.strategies

.. autoclass:: caf.strategies.AlgorithmStrategy
   :members: __init__, run

.. autoclass:: caf.strategies.SingleIOV
   :members:
   :inherited-members:
   :show-inheritance:

State Machine Framework
+++++++++++++++++++++++

.. automodule:: caf.state_machines
   :members:
   :inherited-members:
   :show-inheritance:

Runner Objects
++++++++++++++

.. automodule:: caf.runners
   :members:
   :inherited-members:
   :show-inheritance:

Skimming Events With The SoftwareTriggerResult
==============================================

Each event recorded by Belle II must have passed the High Level Trigger (HLT).
The HLT runs some reconstruction online and applies cuts on the reconstructed objects in order to to make decisions
about which events to keep.
The decisions are made by the SoftwareTrigger Module and stored in the output basf2 file in a SoftwareTriggerResult object.

Certain calibration analyses require broad types of events (di-muon, Bhabha, ...) which can be skimmed out of the basf2 file
by requiring that some HLT trigger decisions were True/False.
The :b2:mod:`TriggerSkim` Module is provided to help extract the decisions stored in the SoftwareTriggerResult object.
You can then use the :py:func:`if_value() <basf2.Module.if_value>` to decide what to do with the returned value of the
:b2:mod:`TriggerSkim` Module.

.. b2-modules::
    :package: calibration
    :modules: TriggerSkim

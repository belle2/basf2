==========
Simulation
==========

.. _Geant4: https://geant4.web.cern.ch/

The core of the simulation library is `Geant4`_, which is an external library. A simulation library module, **FullSim**, was created as a wrapper for the ``basf2`` - Geant4 interface. The default input parameters for FullSim are stored in two places,

* `simulation/scripts/simulation.py <https://stash.desy.de/projects/B2/repos/software/browse/simulation/scripts/simulation.py>`_ and
* `simulation/modules/fullsim/src/FullSimModule.cc <https://stash.desy.de/projects/B2/repos/software/browse/simulation/modules/fullsim/src/FullSimModule.cc>`_.



Functions in simulation.py
==========================

.. automodule:: simulation
   :members:
   :undoc-members:

Physics list for Geant4
=======================

``basf2`` have two options for the input physics list for Geant4. The default one is "FTFP_BERT", the popular physics list for high energy physics. The second one is "Belle2PhysicsList", optimized for the Belle II needs. The details on the "Belle2PhysicsList" are explained below.

.. toctree::

   Belle2PhysicsList 


FullSimTiming
=============

This module is used to count the CPU time consumed by each subdetector during simulation. The external libraries should be compiled in the debug mode for the job, so usually only experts use this module.


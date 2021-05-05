==========
Simulation
==========

.. _Geant4: https://geant4.web.cern.ch/

The (detector) simulation package simulates how MC events appear in the detector (e.g. which 
tracks they leave). MC with detector simulation can then be processed in the same way as raw 
data, allowing for an accurate comparison that takes detector effects into account as well.

The core of the simulation library is `Geant4`_, which is an external library. 
A simulation library module, **FullSim**, was created as a wrapper for the ``basf2`` 
- Geant4 interface. The default input parameters for FullSim are stored in two places,

* `simulation/scripts/simulation.py <https://stash.desy.de/projects/B2/repos/software/browse/simulation/scripts/simulation.py>`_ and
* `simulation/modules/fullsim/src/FullSimModule.cc <https://stash.desy.de/projects/B2/repos/software/browse/simulation/modules/fullsim/src/FullSimModule.cc>`_.

Geant4 version in the externals library
=======================================

The Geant4 version information is stored in the variable ``G4Version`` in the Geant4 source code `global/management/include/G4Version.hh <https://geant4.kek.jp/lxr/source/global/management/include/G4Version.hh>`_.
 
The history of Geant4 version information in the ``basf2`` external library is as follows:


+------------------------+----------------+
|   externals version    | Geant4 version |
+========================+================+
| v01-09-00 and later    |    10.6.1      |
+------------------------+----------------+
| v01-01-01 to v01-08-00 |    10.1.2      |
+------------------------+----------------+
| v00-04-00 to v01-00-00 |     9.6.2      |
+------------------------+----------------+
| v00-02-02 to v00-03-04 |     9.5.1      |
+------------------------+----------------+
| v00-02-00 to v00-02-01 |     9.5.0      |
+------------------------+----------------+
| v00-00-05 to v00-01-05 |     9.4.1      |
+------------------------+----------------+
| v00-00-01 to v00-00-04 |     9.3.0      |
+------------------------+----------------+

To check the externals version of your current ``basf2`` setup, use::

  echo $BELLE2_EXTERNALS_VERSION 


Physics list for Geant4
=======================

PhysicsList collects information on all the particles, physics processes and production thresholds to 
be used in the detector simulation. This class is passed to the Geant4 run manager as the 
"physics configuration” of the application. 
``basf2`` has two options for the input physics list for Geant4. 

The default physics list
""""""""""""""""""""""""

Until November 29, 2013, the default physics list parameter for FullSim was set as *QGSP_VERT*. 
After this day, the parameter was changed to *FTFP_BERT*, the popular physics list for high energy physics. 

Belle2PhysicsList
"""""""""""""""""

The second one is *Belle2PhysicsList*, optimized for the Belle II needs by the Geant4 experts. 
The details on the *Belle2PhysicsList* are explained below. 

.. note:: 

   The **Belle2PhysicsList** option became the default option with basf2 release 05-00-00.


.. toctree::
   Belle2PhysicsList 

Secondary particles
===================

.. toctree::
   Secondary

FullSim
=============

.. toctree::
   FullSim

Functions in simulation.py
==========================

.. automodule:: simulation
   :members:
   :undoc-members:



FullSim
========

``FullSim`` module provides the full *Geant4* simulation for the framework.
It initializes Geant4, calls the converter to translate the **ROOT** TGeo
volumes/materials to native Geant4 volumes/materials, initializes the
physics processes and user actions.

This module requires a valid geometry in memory (gGeoManager). Therefore,
a geometry building module (e.g ``Geometry`` ) should have been executed before this module is called.

.. b2-modules::
   :modules: FullSim
   :noindex:

FullSimTiming
==============

This module is used to count the CPU time consumed by each subdetector during simulation. The externals library should be compiled in the debug mode for the job execution, so usually only experts use this module.

.. b2-modules::
      :modules: FullSimTiming

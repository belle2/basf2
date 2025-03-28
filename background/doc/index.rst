Background Package
===================
In the following we describe the ``background`` package (documentation still under development).

The background package is how we introduce beam background hit rates in MC samples. 
Things users should consider when using this package are the following: 

* Run conditions (i.e. independent vs dependent MC samples): 

  * Run Independent or Simulated Beam Backgrounds: The experiment number used for "Exp: 100X" (ex. 1002 for phase 2, 1003 for phase 3 Run 1, and 1004 for Run 2). 
    The files produced from these 'experiments' are in coordination with the background group and are used in run-independent MC proudction.
  * Run Dependent or Random Trigger Events: These files are used to produce BG overlay files for use in run-dependent MC production. 
    Essentially, a random trigger is used to collect events that don't contain physic signals and therefore represent the effect of beam backgrounds and detector noise.

* Method in which one wants to include backgrounds (i.e. Overlay vs. Mixing)

  * Overlay (default method): Adding raw hits (digits or waveforms) of measured (or simulated) background to simulated physics event. 
    In case of simulated background the raw hits are prepared by background mixing.
  * Mixing: Adding simulated hits of many simulated background events to a single simulated physics event. Each simulated background event is in addition randomly shifted within the given time window. 
    The number of events added is determined from the rate of a particular background type. 
    There are two categories: beam-induced backgrounds (Touschek, beam-gas interactions etc), luminosity backgrounds (radiative Bhabha, two-photon). 
    The former ones are simulated with SAD simulation provided by the Accelerator group and the latter ones within ``basf2`` using the corresponding particle generator. 
    The rates of the beam-induced backgrounds depend on the accelerator lattice, collimator settings, gas pressure and beam currents, while the rates of luminosity backgrounds are proportional to luminosity. 
    Needless to say that this method is slow and very much statistically limited. For the list of background types see framework/dataobjects/include/BackgroundMetaData.h

For details on background overlays for each release, see here: `Data Production Gitlab for BGOverlays <https://gitlab.desy.de/belle2/data-production/data/-/tree/master/BGOverlay?ref_type=heads>`_

For details for run dependent generation, see here: `Data Production Gitlab For MCRD Production <https://gitlab.desy.de/belle2/data-production/data/-/tree/master/mcrd_processing?ref_type=heads>`_

Useful Python Tools: 
---------------------
.. automodule:: background
   :members:


Modules
-------

This is a list of the ``background`` modules. 

.. b2-modules::
   :package: background
   :io-plots:
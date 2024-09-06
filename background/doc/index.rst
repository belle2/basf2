Background module
=================
In the following we describe the ``background`` package (documentation still under development).

The background package is how we introduce beam background hit rates in MC samples. 
Things users should consider when using this module are the following: 
* Run conditions (i.e. indepedent vs dependent MC samples): 

  * Run Indepedent or Simulated Beam Backgrounds: The experiment number used for "Exp: 100X" (ex. 1002 for phase 2 and 1003 for phase 3). The files produced from these 'experiments' are in coordination with the background group and are used in run-independent MC proudction.
  * Run Dependent or Random Trigger Events: These files are used to produce BG overlay files for use in run-dependent MC production. Essentially, a random trigger is used to collect events that don't contain physic signals and therefore represent the effect of beam backgrounds and detector noise.

* Method in which one wants to include backgrounds (i.e. Overlay vs. Mixing)

  * Overlay: The default method of introducing beam background events by overlaying digits or raw hits ontop of simulated events. 
  * Mixing: A different method to introduce beam background events that doesn't require ReadoutFrame (ROF) information. Instead, it includes the background events at the Simulation Hit level and shifts the event type within the given time window. 


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
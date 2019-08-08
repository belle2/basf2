.. _standardparticles:

Standard Particles
==================

The standard particle lists provide recommended selection criteria for final-state particles, and in some cases, for composite particles.
The recommended selections are provided by the performance group(s).

.. warning:: 
        At the moment, the **charged** particle standard lists are not recommended for use with recent data processings.
        This will improve, but for now we recommend only using the standard charged lists in MC studies and for benchmarking.

.. note::
        By contrast, the neutral standard lists are fine for data.

Although not available with this release, systematics will be provided centrally for these recommended lists.
For information about the status of this, please see `BII-4105`_.

.. _BII-4105: https://agira.desy.de/browse/BII-4105

There are also some skimming, development and legacy lists available.
These are specifically for use in skims, for study, or for comparison with Belle and old simulations.
They are not recommended for normal analysis use (unless you are working on a skim or legacy study).

Recommended final-state particle list builder functions
-------------------------------------------------------

.. autofunction:: stdPhotons.stdPhotons
.. autofunction:: stdPi0s.stdPi0s

.. autofunction:: stdV0s.stdKshorts
.. autofunction:: stdV0s.stdLambdas

Not (yet) recommended charged particle standard lists
-----------------------------------------------------

.. warning:: 
        Unfortunately these lists are not yet recommended for use with recent data processings.

.. autofunction:: stdCharged.stdPi
.. autofunction:: stdCharged.stdK
.. autofunction:: stdCharged.stdPr
.. autofunction:: stdCharged.stdE
.. autofunction:: stdCharged.stdMu


Other functions available
-------------------------

These other functions are not recommended for normal use without some study of the selection, or if you are working on skimming.
If you improve these lists, please report in a performance meeting and make a pull request.

.. automodule:: stdCharged
   :members:
   :undoc-members:
   :exclude-members: stdPi, stdK, stdPr, stdE, stdMu

.. automodule:: stdPhotons
   :members:
   :undoc-members:
   :exclude-members: stdPhotons

.. automodule:: stdPi0s
   :members:
   :undoc-members:
   :exclude-members: stdPi0s

.. automodule:: stdV0s
   :members:
   :undoc-members:
   :exclude-members: stdKshorts, stdLambdas

.. automodule:: stdKlongs
   :members:
   :undoc-members:


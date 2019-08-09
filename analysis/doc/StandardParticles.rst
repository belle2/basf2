.. _standardparticles:

Standard Particles
==================

.. warning:: 
        At the moment, the standard particle lists are **not** recommended for use with recent data processings.
        This will improve, but for now we recommend only using the standard charged lists in MC studies and for benchmarking.

The goal is that the standard particle lists will provide recommended selection criteria for final-state particles, and in some cases, for composite particles.
The recommended selections will be provided by the performance group(s).
Furthermore systematics will be provided centrally for these recommended lists.
For information about their status, please see :issue:`BII-4105`.


There are also some skimming, development and legacy lists available.
These are specifically for use in skims, for study, or for comparison with Belle and old simulations.

.. when the standard lists get useable, the skim lists will still not be recommended so uncomment the line below
.. They are not recommended for normal analysis use (unless you are working on a skim or legacy study).

At the moment they are only to be used for benchmarking, and for studies in MC.

Not (yet) recommended final-state particle list builder functions
-----------------------------------------------------------------

.. warning:: 
        Unfortunately these lists are not yet recommended for use with recent data processings.

.. autofunction:: stdPhotons.stdPhotons
.. autofunction:: stdPi0s.stdPi0s

.. autofunction:: stdV0s.stdKshorts
.. autofunction:: stdV0s.stdLambdas

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


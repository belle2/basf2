.. _standardparticles:

Standard Particles
==================

The standard particle lists provide recommended selection criteria for final-state particles, and in some cases, for composite particles.
The recommended selections are provided by the physics performance group.

Although not available yet, it is envisaged that systematics will be provided centrally for these recommended lists.

There are also some skimming, development and legacy lists available.
These are specifically for use in skims, for study, or for comparison with Belle and old simulations.
They are not recommended for normal analysis use (unless you are working on a skim or legacy study).
In addition, some of the light meson standard particle lists are under development.
If you are interested in light mesons you could consider presenting your study in a physics performance meeting.

Recommended final-state particle list builder functions
-------------------------------------------------------

.. autofunction:: stdPhotons.stdPhotons
.. autofunction:: stdPhotons.stdPi0s
.. autofunction:: stdCharged.stdPi
.. autofunction:: stdCharged.stdK
.. autofunction:: stdCharged.stdPr
.. autofunction:: stdCharged.stdE
.. autofunction:: stdCharged.stdMu
.. autofunction:: stdV0s.stdKshorts
.. autofunction:: stdV0s.stdLambdas


Other functions available
-------------------------

These other functions are not recommended for normal use without some study of the selection, or if you are working on skimming.
If you improve these lists, please report in a physics performance meeting and make a pull request.

.. automodule:: stdCharged
   :members:
   :undoc-members:
   :exclude-members: stdPi, stdK, stdPr, stdE, stdMu

.. automodule:: stdCharm
   :members:
   :undoc-members:

.. automodule:: stdDiLeptons
   :members:
   :undoc-members:

.. automodule:: stdLightMesons
   :members:
   :undoc-members:

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


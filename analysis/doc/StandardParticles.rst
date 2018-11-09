.. _standardparticles:

Standard Particles
==================

The standard particle lists provide recommended selection criteria for final-state particles, and some cases, for composite particles.
The recommended selections are studied and provided by the physics performance group.

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
   :undoc-members:

.. automodule:: stdCharm
   :undoc-members:

.. automodule:: stdDiLeptons
   :undoc-members:

.. automodule:: stdLightMesons
   :undoc-members:

.. automodule:: stdPhotons
   :undoc-members:

.. automodule:: stdPi0s
   :undoc-members:

.. automodule:: stdV0s
   :undoc-members:

.. automodule:: stdKlongs
   :undoc-members:


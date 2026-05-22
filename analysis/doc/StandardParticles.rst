.. _standardparticles:

Standard Particles
==================

.. deprecated:: light-2604-jellyfish
        `stdCharged.stdE`, `stdCharged.stdMu`, `stdCharged.stdLep`, and most lists in
        `stdPhotons` and `stdPi0s` are deprecated and will be removed at the end of 2026.
        Use the following replacements or the 'all' list instead:

        - `stdCharged.stdCharged` with ``listtype='clean'``:
          ``thetaInCDCAcceptance and dr < 0.5 and abs(dz) < 2``
        - `stdPhotons.stdPhotons` with ``listtype='base'``:
          ``inCDCAcceptance and abs(clusterTiming) < 200``
        - `stdPi0s.stdPi0s` with ``listtype='base'``:
          reconstructed from two ``gamma:base`` photons with no additional cuts

        These lists provide a common baseline; further selections should be optimised
        based on the requirements of each individual analysis.
        Please refer to the
        `Performance Recommendations <https://belle2.pages.desy.de/performance/recommendations/>`_
        for further guidance. To provide feedback on the removal, see
        `work item #11641 <https://gitlab.desy.de/belle2/software/basf2/-/work_items/11641>`_.

.. warning::
        Some standard particle lists are outdated and no longer recommended for use.
        Please refer to the
        `Performance Recommendations <https://belle2.pages.desy.de/performance/recommendations/>`_
        for recommended selections and the latest updates on what to use.

.. tip::
        The following ``V0`` standard lists are **good** (i.e. recommended for use):

        * `stdV0s.stdKshorts`
        * `stdV0s.stdLambdas`

These standard particle lists provide common selection criteria for final-state particles, 
and in some cases, for composite particles.
There are also some skimming, development and legacy lists available.
These are specifically for use in skims, for study, or for comparison with Belle and old simulations.
If you don't know that you specifically need these legacy/skim lists, then avoid them.

Default final-state particle list builder functions
---------------------------------------------------

.. autofunction:: stdPi0s.stdPi0s
.. autofunction:: stdV0s.stdKshorts
.. autofunction:: stdV0s.stdLambdas
.. autofunction:: stdCharged.stdE
.. autofunction:: stdCharged.stdMu
.. autofunction:: stdCharged.stdLep

Not (yet) recommended final-state particle list builder functions
-----------------------------------------------------------------

.. warning::
        Unfortunately these lists are not yet recommended for use with recent data processings.

.. autofunction:: stdPhotons.stdPhotons
.. autofunction:: stdCharged.stdPi
.. autofunction:: stdCharged.stdK
.. autofunction:: stdCharged.stdPr


Other functions available
-------------------------

.. warning::
        These other functions are not recommended for normal use without some study of the selection, or if you are working on skimming.
        If you use and improve these lists, please report in a performance meeting and make a merge request.

.. automodule:: stdCharged
   :members:
   :undoc-members:
   :exclude-members: stdPi, stdK, stdPr, stdE, stdMu, stdLep

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

.. automodule:: stdHyperons
   :members:
   :undoc-members:

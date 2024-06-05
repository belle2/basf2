.. _standardparticles:

Standard Particles
==================

.. warning::
        Construction site!

        At the moment, we are in a strange situation where some of the standard particle lists are actually
        **not** recommended for use with recent data processings.

        However, some of these lists *have* been tested and *can* be used.

.. tip::
        The following ``pi0`` and ``V0`` standard lists are **good** (i.e. recommended for use):

        * `stdPi0s.stdPi0s`
        * `stdV0s.stdKshorts`
        * `stdV0s.stdLambdas`

        However, please check
        `this Neutrals Performance confluence page <https://confluence.desy.de/display/BI/Neutrals+Performance>`_
        for the latest updates.

.. tip::
        The following standard lepton lists are **good** (i.e. recommended for use):

        * `stdCharged.stdE`
        * `stdCharged.stdMu`

	for a choice of the input ``listtype`` parameter among:

	* 'FixedThresh05'
	* 'FixedThresh09'
	* 'FixedThresh095'
	* 'UniformEff60'
	* 'UniformEff70'
	* 'UniformEff80'
	* 'UniformEff90'
	* 'UniformEff95'

        However, please check
        `the Lepton ID Performance confluence page <https://confluence.desy.de/display/BI/Lepton+ID+Performance>`_
        for the latest updates.


The ultimate goal is that these standard particle lists will provide recommended selection
criteria for final-state particles, and in some cases, for composite particles.
The recommended selections will be provided by the performance group(s).
Furthermore the intention is that systematics will be provided centrally for these recommended lists.
For information about their status, please see :issue:`4045`.

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

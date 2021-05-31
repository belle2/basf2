.. _cut_strings_selections:

Cut strings and selections
--------------------------

Cut strings in ``basf2`` analysis scripts work quite intuitively, particularly if you're comfortable with python logic.
There are some minor differences with respect to  `ROOT <https://root.cern.ch>`_ cut strings.

Cut strings are used to manipulate candidates (i.e. `ParticleList`_ s) with `modularAnalysis.applyCuts`, and `modularAnalysis.cutAndCopyList`.
Events selections are applied with `modularAnalysis.applyEventCuts`.

.. _ParticleList: https://b2-master.belle2.org/software/development/classBelle2_1_1ParticleList.html

Allowed comparisons are ``<``, ``<=``, ``>``, ``>=``, ``==`` and ``!=``.
Two-sided inequalities like ``1.2 < M < 1.5`` also work as expected.
Use the logical operators ``and`` and ``or`` to combine your cuts.

.. warning:: You should use square brackets ``[``, ``]`` to separate conditional statements.

.. hint::

   For a more in-depth documentation of cut strings for developers, you can refer to the `doxygen documentation`_
   for the ``GeneralCut`` class.

.. _doxygen documentation: https://b2-master.belle2.org/software/|release|/classBelle2_1_1GeneralCut.html

This logic can become quite powerful, particularly in combination with the :b2:var:`formula` MetaVariable.
Here are some examples:

.. code-block:: python

     from modularAnalysis import applyEventCuts, fillParticleList

     total_ecl_clusters = "formula(nParticlesInList(gamma:all) + nParticlesInList(e+:clusters))"
     applyEventCuts("[nTracks > 10] and [ %s > 4]" % total_ecl_clusters, path=mypath)
     fillParticleList("pi+:highEForward", "[E > 1] and [theta < 1.0]" path=mypath)

     # WILL NOT WORK!!!
     applyEventCuts("(nTracks > 10) and (nParticlesInList(gamma:all) > 4)", path=mypath)

The above example can be made even more readable with the use of variable aliases (`VariableManager.addAlias`).

.. code-block:: python

     from modularAnalysis import applyEventCuts
     from variables import variables as vm

     vm.addAlias("totalECLClusters", "formula(nParticlesInList(gamma:all) + nParticlesInList(e+:clusters))")
     applyEventCuts("[nTracks > 10] and [totalECLClusters > 4]", path=mypath)

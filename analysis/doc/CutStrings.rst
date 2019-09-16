Cut strings and selections
--------------------------

Cut strings in ``basf2`` analysis scripts work quite intuitively.
If you are familiar with `ROOT <https://root.cern.ch>`_ cutstrings and/or python logic, you will probably not notice much difference.

Cut strings are used to manipulate candidates (i.e. `ParticleList`_ s) with `modularAnalysis.applyCuts`, and `modularAnalysis.cutAndCopyList`.
Events selections are applied with `modularAnalysis.applyEventCuts`.

.. _ParticleList: https://b2-master.belle2.org/software/development/classBelle2_1_1ParticleList.html

You can use logical operators ``&&``, ``||`` or words ``and``, ``or`` in your cuts.

.. warning:: You should use square braces ``[``, ``]`` to separate conditional statements.

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

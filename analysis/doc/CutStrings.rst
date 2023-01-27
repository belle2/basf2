.. _cut_strings_selections:

Cut strings and selections
--------------------------

Cut strings in ``basf2`` analysis scripts work quite intuitively, particularly if you're comfortable with python logic.
There are some minor differences with respect to  `ROOT <https://root.cern.ch>`_ cut strings.

Cut strings are used to manipulate candidates (i.e. `ParticleList`_ s) with `modularAnalysis.applyCuts`, and `modularAnalysis.cutAndCopyList`.
Events selections are applied with `modularAnalysis.applyEventCuts`.

.. _ParticleList: https://software.belle2.org/development/classBelle2_1_1ParticleList.html

Allowed comparisons are ``<``, ``<=``, ``>``, ``>=``, ``==`` and ``!=``.
Two-sided inequalities like ``1.2 < M < 1.5`` also work as expected.
You can use logical operators ``and`` and ``or`` to combine your cuts into more complex conditions.
Use ``not`` to negate conditional statements.
Square brackets ``[``, ``]`` are for grouping conditional statements.

Cut strings support writing formulas in cuts directly. Operator precedences work as expected. 
Formulas in conditions can be written freely e.g ``0.2 <= dx**2+dy**2 <= 0.7`` as one would expect in python.
Common arithmetic operators ``+``, ``-``, ``*``, ``/`` can be used. For the exponentiation operator both ``**`` and ``^`` are valid.
You can use parenthesis ``(`` ``)`` to group expressions in formulas.

Formulas as arguments for MetaVariables are also supported e.g ``abs(dx+dy) < 1`` .

Cuts supports writing numeric literals of different data types in different formats. Double values can be written in scientific notation. 
You can also compare to special values like ``nan`` and ``inf``. Integer literals can also be given in hexadecimal notation e.g ``0xABC`` (case insensitive).
Boolean literals can be written in both python ``True/False`` and c++ ``true/false`` style.

.. warning:: You should not use square brackets ``[``, ``]`` for grouping expressions in formulas. Use parenthesis ``(`` ``)`` only.

.. hint::

     Be aware of operator precedence ``not > and > or`` when combining your statements
     e.g ``condition1 and condition2 or condition3`` is evaluated equivalent to ``[condition1 and condition2] or condition3``,
     ``not condition1 and condition2`` is evaluated equivalent to ``[not condition1] and condition2``.
     If necessary, square brackets can be used to group conditional statements and apply operators as desired.


.. hint::

   For a more in-depth documentation of cut strings for developers, you can refer to the `doxygen documentation`_
   for the ``GeneralCut`` class.

.. _doxygen documentation: https://software.belle2.org/|release|/classBelle2_1_1GeneralCut.html

.. hint:: In previous versions the `formula` MetaVariable was needed to support formulas in cuts. This is still supported but not necessary anymore.

This logic can become quite powerful.
Here are some examples.

.. code-block:: python

     from modularAnalysis import applyEventCuts, fillParticleLists
     fillParticleLists([("gamma:allecl", "isFromECL"), ("e+:clusters", "clusterE > 0")], path=mypath)

     total_ecl_clusters = "nParticlesInList(gamma:allecl) + nParticlesInList(e+:clusters)"
     applyEventCuts(f"[nTracks > 10] and [{total_ecl_clusters} > 4]", path=mypath)

The above example can be made even more readable with the use of variable aliases (`VariableManager.addAlias`).

.. code-block:: python

     from modularAnalysis import applyEventCuts
     from variables import variables as vm

     vm.addAlias("totalECLClusters", "nParticlesInList(gamma:all) + nParticlesInList(e+:clusters)")
     applyEventCuts("[nTracks > 10] and [totalECLClusters > 4]", path=mypath)

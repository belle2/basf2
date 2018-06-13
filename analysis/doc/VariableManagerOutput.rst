Variable Manager Output 
=======================

A common task is to write out quantities from the VariableManager to a `ROOT`_ file to analyse it outside of the ``basf2`` framework (e.g. to perform a fit, and extract a physical observable). 

.. _ROOT: https://root.cern.ch

There are several modules which write out Variables in one form or another:

VariablesToHistogram
--------------------

Writes out Variables to a `ROOT`_ TH1F or TH2F histogram.

.. code-block:: python

        from modularAnalysis import variablesToHistogram
        list_of_variables_and_bins = [
                ('pt', 100, 0, 1),
                ('E', 100, 0, 4)
        ]
        variablesToHistogram('pi+:all', list_of_variables_and_bins)


VariablesToNtuple
-----------------

Writes out Variables to a flat `ROOT`_ TTree.
For each candidate in the given ParticleList, there will be one entry in the TTree containing the desired Variables. 
In other words, this produces a candidate-based ROOT file.

.. code-block:: python
 
        from modularAnalysis import variablesToNtuple
        list_of_interesting_variables = [
                'expNum', 'runNum', 'evtNum', 
                'E', 'px', 'py', 'pz', 'isSignal', 
        ]
        variablesToNtuple('pi+:all', list_of_interesting_variables)



VariablesToEventBasedTree 
-------------------------

Writes out Variables to a structured `ROOT`_ TTree.
For each **event** an entry is written into the Tree containing one array (for each Particle in the ParticleList) for each Variable.

.. seealso:: More information for working event-wise is given in the section about `EventBased`.

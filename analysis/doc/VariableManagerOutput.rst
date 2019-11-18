.. _variablemanageroutput:

Variable Manager Output 
=======================

A common task is to write out information to a `ROOT`_ file to analyse it outside of the ``basf2`` framework (e.g. to perform a fit, and extract a physical observable). 

.. _ROOT: https://root.cern.ch

There are several modules which write out variables in one form or another.
And several helpful functions to set up your output TTree in a format you like.

.. _v2nt:

VariablesToNtuple
-----------------

Writes out Variables to a flat `ROOT`_ TTree (it is also possible to write out several trees to one file).

.. note:: 
        New since release-03: event, run, and experiment numbers are now automatically included. 
        If you have are writing candidates, you will also see a candidate counter and the number of candidates (ncandidates).


Candidate-wise
~~~~~~~~~~~~~~
For each candidate in the given `ParticleList`_, there will be one entry in the TTree containing the desired Variables. 
In other words, this produces a candidate-based ROOT file.
Here is an example of use:

.. _ParticleList: https://b2-master.belle2.org/software/development/classBelle2_1_1ParticleList.html

.. code-block:: python
 
        from modularAnalysis import variablesToNtuple
        list_of_interesting_variables = [
                'E', 'px', 'py', 'pz', 'isSignal',  # related to the candidate
                'nTracks', # related to the event
        ]
        variablesToNtuple('pi+:all', list_of_interesting_variables, path=mypath)

Event-wise
~~~~~~~~~~
This module will also work even when provided with *no* ParticleList name.
In this case it will be filled once per event.
Here is an example of event-wise usage:

.. code-block:: python

        from modularAnalysis import variablesToNtuple
        list_of_interesting_event_variables = [
                'L1Trigger', 'HighLevelTrigger', 'nTracks' # purely event
        ]
        variablesToNtuple('', list_of_interesting_event_variables, path=mypath)


Multiple TTress
~~~~~~~~~~~~~~~

You can write several trees to the same file by calling the module several times with the different ``treename`` and the same ``filename``.

.. code-block:: python

        from modularAnalysis import variablesToNtuple
        variablesToNtuple('', list_of_interesting_event_variables, 
                          treename="event", filename="myoutput.root", 
                          path=mypath)
        variablesToNtuple('pi+:all', list_of_interesting_variables, 
                          treename='pions', filename='myoutput.root', # <-- same file
                          path=mypath)
        variablesToNtuple('K+:all', list_of_interesting_variables, 
                          treename='kaons', filename='anotheroutput.root', # <-- different file
                          path=mypath)


As with many modules, there is a `modularAnalysis` convenience function:

.. autofunction:: modularAnalysis.variablesToNtuple
   :noindex:

.. _v2ebt:

VariablesToEventBasedTree 
-------------------------

Writes out Variables to a structured `ROOT`_ TTree.
For each **event** an entry is written into the Tree containing one array (for each Particle in the ParticleList) for each Variable.

.. seealso:: More information for working event-wise is given in the section about :doc:`EventBased`.

VariablesToHistogram
--------------------

Writes out Variables to a `ROOT`_ TH1F or TH2F histogram.
Here is an example of use:

.. code-block:: python

        from modularAnalysis import variablesToHistogram
        list_of_variables_and_bins = [
                ('pt', 100, 0, 1),
                ('E', 100, 0, 4)
        ]
        variablesToHistogram('pi+:all', list_of_variables_and_bins)

Here is the full function documentation of the `modularAnalysis` convenience function:

.. autofunction:: modularAnalysis.variablesToHistogram
   :noindex:

.. _v2hdf5:

VariablesToHDF5
---------------

Writes out variables to a flat HDF5 format file (for use with `pandas.DataFrame`_ tools).
Analogous to `VariablesToNtuple <v2nt>`.


.. _pandas.DataFrame: https://pandas.pydata.org

.. note:: 
        There is currently no `modularAnalysis` convenience function. 
        Instead you can add the module to your path explicitly (it only takes two lines).

.. code-block:: python
 
        from b2pandas_utils import VariablesToHDF5
        v2hdf5 = VariablesToHDF5("pi+:all", list_of_interesting_variables,
                                 filename="variables.hdf5")
        mypath.add_module(v2hdf5)

.. autoclass:: b2pandas_utils.VariablesToHDF5
   :members:


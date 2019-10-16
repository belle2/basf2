
.. _eventbased:

Event based analysis
====================

.. note::
        The analysis software framework is designed to work with particles.
        In the default candidate-wise paradigm the analyst takes lists of particles, builds up candidates, loads analysis modules,
        and saves information (i.e. using the `VariablesToNtuple <v2nt>` tool) for each candidate.
        If you are new, it is probably best to start with this approach.

In some situations, it is useful to make cuts on, and write out event-wise information.

.. warning::
        Please note that this use model and the documentation are under development. 

Please ask questions at `Belle II questions <https://questions.belle2.org/>`_ and report bugs using `JIRA <https://agira.desy.de/projects/BII/>`_. 
Be sure to tag your questions and bug reports as "analysis" and "event based" to help us find them.


VariablesToEventBasedTree
-------------------------

The `VariablesToNtuple <v2nt>` tool writes out one entry in a TTree for every candidate. If you want one entry for every event, you should use the VariableToEventBasedTree module. Note that this still operates on a particle list so you will still need to build up a list of candidates in the normal way.

Here is an example of writing event-wise information for all pions in the event:

.. code-block:: python

    import basf2
    from stdCharged import stdPi
     
    mypath = basf2.Path()
    stdPi('all', path=mypath)
    mypath.add_module('VariablesToEventBasedTree',
                      particleList='pi+:all',
                      variables=['isSignal', 'mcErrors', 'p', 'E'],
                      event_variables=['nTracks', 'expNum', 'runNum', 'evtNum'])


.. b2-modules::
   :modules: VariablesToEventBasedTree
   :noindex:

Event based selection
---------------------

Instead of making cuts on properties of particle lists, it is possible to reject all events that fail an event based variable cut.
Here is a simple example:

.. code-block:: python

    from modularAnalysis import applyEventCuts
    from stdCharged import stdPi
     
    stdPi('all')
    applyEventCuts('R2EventLevel < 0.3')

This builds the list of all pions but only processes events where the event-level R2 passes the cut. 
.. FIXME B2A307 is broken, when it's fixed, please uncomment this line (BII-5330)
.. See :download:`B2A307 </analysis/examples/tutorials/B2A307-BasicEventWiseNtupleSelection.py>` for a full example.

.. autofunction:: modularAnalysis.applyEventCuts
   :noindex:

Event based variables
~~~~~~~~~~~~~~~~~~~~~

Here is a list of event based variables on which you can make a cut using the above.

.. b2-variables::
   :description-regex-filter: ^\[Eventbased\].*
   :noindex:


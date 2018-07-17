=========
Analysis
=========

The analysis package collects modules and tools for high-level analysis of Belle II data.
With analysis modules you are able to open mdst format files, create final state particles, and build more complicated decays.
With these particle decay candidates, you can use high level tools such as `TreeFitter`, `FlavorTagger`, and the `FullEventInterpretation` (FEI).

The core data objects of the ``basf2`` analysis package, are the `Particle`_ class, and the `ParticleList`_ class.
Almost all analysis modules are concerned with the manipulation of `ParticleList`_ s.

Examples of analysis steering files can be found in tutorial directory of analysis package::

        ls $BELLE2_RELEASE_DIR/analysis/examples/tutorials

.. _ParticleList: https://b2-master.belle2.org/software/development/classBelle2_1_1ParticleList.html

.. _Particle: https://b2-master.belle2.org/software/development/classBelle2_1_1Particle.html


Particles
=========

Particles in basf2 are created from mdst dataobjects: Tracks, tracking vertices (V0s) ECLClusters, and KLMClusters. 
The `Particle`_ class, provides a unified way to treat different objects at a high level interpreting them as phsical particles.
All of the `Particle`_ s in an event that fall within certain criteria are collected into `ParticleList`_ s.

The Belle II physics performance group already has several predefined `ParticleList`_ s.
To use them you need to use functions described in the standard particle lists.

.. toctree:: StandardParticles

Creation of new particle lists by combination of particles is essential part of almost any analysis job.
The syntax used during this procedure is the `DecayString` and is described below:

.. toctree:: DecayString


Modular analysis convenience functions
======================================

The general idea of ``basf2`` modules is described in the section on :ref:`general_modpath`.
The analysis package provides pre-defined python wrapper functions designed to simplify users python code.

.. toctree:: MAWrappers

There are also some more specialised python wrapper functions for vertex-fitting and for dealing with photons.

.. toctree:: Vertex
.. toctree:: PhotonWrappers

Variables
=========

While ``basf2`` operates on `ParticleList`_ s, it is also important to calculate variables associated with a given candidate or event.
Here is a list of variables known to ``basf2`` that can be used in cut strings or written out to file.

.. toctree:: Variables

Output for offline analysis
===========================

For historical reasons, there are two ways to save information from analysis jobs (e.g. for offline analysis using python tools such as `pandas`_)
The recommended method is to get output directly from the VariableManger.
New users are encouraged to start with this.

.. _pandas: https://pandas.pydata.org/

.. toctree:: VariableManagerOutput

We also support `NtupleMaker` tools which were developed first.
However we expect to deprecate this method in the next release, and eventually remove.
The documentation is included here for backward compatibility.

.. toctree:: NtupleMaker

Event-based analysis
====================

.. toctree:: EventBased

Truth-matching
==============

.. toctree:: MCMatching

How to Veto
===========

.. toctree:: HowToVeto

Advanced Topics
===============

.. toctree:: RestOfEvent

.. toctree:: FlavorTagger

.. toctree:: FullEventInterpretation

.. toctree:: ContinuumSuppression

.. toctree:: TreeFitter

.. toctree:: VertexFitter

.. toctree:: MVA


.. _analysismodules:

Full list of analysis modules
=============================

The Belle II physics analysis software follows the logic of the general Belle II analysis softwareframework (BASF2) where independent modules solve relatively small problems, where practically all communication between modules happens in the form of data objects. Modules are executed in a linear fashion inside a module path, and can make objects available “downstream” by saving them in the DataStore.

Typical physics analysis performed at B factories can be separated into well defined analysis actions. For example, in a measurement of time-dependent CP violation in B0 → φKS0 decays we need to execute the following actions:

#. create list of charged kaon candidates,
#. create list of φ candidates by making combinations of two oppositely charged kaons,
#. create list of KS0 candidates,
#. create list of B0 candidates by making combinations of φ and KS0 candidates,
#. calculate continuum suppression variables,
#. determine the flavor of B0 candidates,
#. determine the decay vertex of B0 candidates,
#. determine the decay vertex of the other B meson in the event,
#. write out all relevant info to ntuple for offline analysis.


A measurement of time-dependent CP violation in B0 decays to a different final state, e.g. :math:`J/\psi K_S^0,\ D^+D^−` or :math:`K^{*0}\gamma`, would consist of conceptually identical analysis actions. Even within the same analysis sequence the same analysis action can appear several times. In the example above, the actions 2 and 4 are conceptually the same. In both cases new particle is created by combining others. The only difference is in the input. The only difference between differnet decay modes mentioned above is in the intermediate and final state particles. All other steps, like 5, 6, 7, 8, and 9 are in principle the same. The input is of course different, but the result is the same. Therefore, commonly used analysis tools need to be prepared in order to enable efficient and accurate data analysis. Each analysis action can then be performed by a separate analysis module, combined into an analysis sequence steered by a python script.



.. b2-modules::
   :package: analysis

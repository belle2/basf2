.. _analysis:

========
Analysis
========

The analysis package collects modules and tools for high-level analysis of Belle II data.
With analysis modules you are able to open mdst format files, create final state particles, and build more complicated decays.
With these particle decay candidates, you can use high level tools such as :doc:`TreeFitter`, :doc:`FlavorTagger`, and the :doc:`FullEventInterpretation` (FEI).

.. tip:: For a series of hands-on introductory lessons, take a look at :numref:`onlinebook_basf2`.

The core data objects of the ``basf2`` analysis package, are the `Particle`_ class, and the `ParticleList`_ class.
Almost all analysis modules are concerned with the manipulation of `ParticleList`_ s.

Examples of analysis steering files can be found in tutorial directory of analysis package::

        ls $BELLE2_RELEASE_DIR/analysis/examples/tutorials

.. _ParticleList: https://software.belle2.org/|release|/classBelle2_1_1ParticleList.html

.. _Particle: https://software.belle2.org/|release|/classBelle2_1_1Particle.html


.. _analysis_particles:

Particles
=========

Particles in basf2 are created from reconstructed dataobjects: Tracks, tracking vertices (V0s), ECLClusters, and KLMClusters. 
The `Particle`_ class, provides a unified way to treat different objects at a high level interpreting them as phsical particles.
All of the `Particle`_ s in an event that fall within certain criteria are collected into `ParticleList`_ s.

The Belle II physics performance group already has several predefined `ParticleList`_ s.
To use them you need to use functions described in the standard particle lists.

.. toctree:: StandardParticles

Creation of new particle lists by combination of particles is essential part of almost any analysis job.
The syntax used during this procedure is the :doc:`DecayString` and is described below:

.. toctree:: DecayString

It is often desirable to apply selections to `ParticleList`_ s, and events.
This is done with cut strings.

.. toctree:: CutStrings


Modular analysis convenience functions
======================================

The general idea of ``basf2`` modules is described in the section on :ref:`general_modpath`.
The analysis package provides pre-defined python wrapper functions designed to simplify users python code.
For vertex-fitter functions see the later section on :ref:`analysis_vertexfitting`.

.. toctree:: MAWrappers

There are also some more specialised python wrapper functions for dealing with photons.

.. toctree:: PhotonWrappers
.. toctree:: KinFit


.. _analysis_variables:

Variables
=========

While ``basf2`` operates on `ParticleList`_ s, it is also important to calculate physics quantities associated with a given candidate or event.

In ``basf2`` analysis, variables are handled by the `VariableManager`.
There are many variables available for use in analysis.
Probably the most obvious, and useful are: :b2:var:`p`, :b2:var:`E`, :b2:var:`Mbc`, and :b2:var:`deltaE`.

You can search the variables in an alphabetical :ref:`b2-varindex`, or browse :ref:`variablesByGroup`.

.. toctree:: Variables


Output for offline analysis
===========================

You can save information from analysis jobs directly from the ``VariableManager`` (e.g. for offline analysis using python tools such as `pandas`_) 

.. _pandas: https://pandas.pydata.org/

.. toctree:: VariableManagerOutput

(Advanced) user mDST files
--------------------------

You can also save events to your own mDST or user-defined dst (uDST) output files.
These files are also created by the :ref:`skim` but the functional code is part of :ref:`analysis`.

.. toctree:: UdstOutput

.. _analysis_vertexfitting:

Vertex (and kinematic) fitting
==============================

There are a number of vertex-fitting and kinematic fitter libraries interfaced and available for use in basf2.

By default, `TreeFitter` is recommended even for simple single-vertex use-cases, however there are use cases where a better-performing or specialist tool is needed.

.. toctree:: TreeFitter

.. toctree:: OtherVertexFitters

These fitters can be added to your basf2 path using a set of the convenient pre-defined python wrapper functions.

.. toctree:: VertexWrappers

Event-based analysis
====================

.. toctree:: EventBased

Truth-matching
==============

.. toctree:: MCMatching

Advanced Topics
===============

.. toctree:: RestOfEvent

.. toctree:: HowToVeto

.. toctree:: FlavorTagger

.. toctree:: DeepFlavorTagger

.. toctree:: FullEventInterpretation

.. toctree:: EventShape

.. toctree:: ContinuumSuppression

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


A measurement of time-dependent CP violation in B0 decays to a different final state, e.g. :math:`J/\psi K_S^0,\ D^+D^−` or :math:`K^{*0}\gamma`, would consist of conceptually identical analysis actions. Even within the same analysis sequence the same analysis action can appear several times. In the example above, the actions 2 and 4 are conceptually the same. In both cases new particle is created by combining others. The only difference is in the input. The only difference between different decay modes mentioned above is in the intermediate and final state particles. All other steps, like 5, 6, 7, 8, and 9 are in principle the same. The input is of course different, but the result is the same. Therefore, commonly used analysis tools need to be prepared in order to enable efficient and accurate data analysis. Each analysis action can then be performed by a separate analysis module, combined into an analysis sequence steered by a python script.



.. b2-modules::
   :package: analysis

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

.. toctree::
   :maxdepth: 2
   :glob:

   Particles
   PythonWrappers
   Variables
   Output
   VertexFitting
   EventBased
   MCMatching
   AdvancedTopics
   AnalysisModules
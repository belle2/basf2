=========
Analysis
=========

Here is containted information about the basf2 analysis software.

.. seealso:: BASF stands for Belle Analysis Software Framework, and number 2 reffers to the Belle2 experiment. The original basf belongs to Belle and is very different from bsaf2.

During the analysis job particle lists are consequently modified by means of the modules, as defined in python script called steering file. Examples of steering files can be found in tutorial directory of analysis package (``analysis/examples/tutorials/``)

.. seealso:: We also tell how to write steering files during tutorial session. You can find materials at Jupyter hub: <https://jupyterhub.belle2.org/hub/spawn>

Particles
=========
 
The subject of the basf2 analysis are objects of class `Particle`_ , that are connected into the objects of class `ParticleList`_ .

In Belle II, we already have several predefined lists of particles. To use them in your steering files, one needs to use functions described in a link below

.. toctree:: StandardParticles

Creation of a new particle list by combination of particles is essemtail part of almost any analysis job. Syntax using during this procedure is describted below

.. toctree:: DecayString

.. _ParticleList: https://b2-master.belle2.org/software/development/classBelle2_1_1ParticleList.html

.. _Particle: https://b2-master.belle2.org/software/development/classBelle2_1_1Particle.html

Modules
=======

The general idea of modular analysis described above: :ref:`general_modpath`. :ref:`Analysis modules <analysismodules>` are written in C++ and can be added directrly to the path, but for the most of them there are pre-defined pyton wrappers that simplify user interface:

.. toctree:: MAWrappers

.. toctree:: Vertex

Variables
=========


While basf2 operates with particles, in the end analysist operates with veriables associated with given particle or event. List of variables that are already known by basf2 and details of how they are handled are given below:

.. toctree:: Variables



Event-based analysis
====================

.. toctree:: Ev-based

Truth-matching
==============

.. toctree:: MCMatching

NTuple creation
===============

.. toctree:: Ntuplemaker

Advanced Topics
===============

.. toctree:: Roe

.. toctree:: Flavortagger

.. toctree:: Fei

.. toctree:: Cs

.. toctree:: Treefitter

.. toctree:: Basf2tmva


.. _analysismodules:

Full list of analysis modules
=============================
.. b2-modules::
   :package: analysis
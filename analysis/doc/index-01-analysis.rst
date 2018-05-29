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
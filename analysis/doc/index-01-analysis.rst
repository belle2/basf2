=========
Analysis
=========

modularAnalysis
===============

.. automodule:: modularAnalysis
   :members:

   .. data:: modularAnalysis.analysis_main

      The 'path' argument of all functions has a default value 'analysis_main',
      which is created implictly. In case you don't want to use this default
      `Path` object, you can do

          >>> import analysisPath
          >>> analysisPath.analysis_main = None

      *before* importing modularAnalysis. This will force you to always specify
      your own path.

vertex
======
.. automodule:: vertex
   :members:
   :undoc-members:


Standard Particles
==================
.. automodule:: stdCharged
   :members:
   :undoc-members:

.. automodule:: stdCharm
   :members:
   :undoc-members:

.. automodule:: stdDiLeptons
   :members:
   :undoc-members:

.. automodule:: stdFSParticles
   :members:
   :undoc-members:

.. automodule:: stdLightMesons
   :members:
   :undoc-members:

.. automodule:: stdPhotons
   :members:
   :undoc-members:

.. automodule:: stdPi0s
   :members:
   :undoc-members:

.. automodule:: stdV0s
   :members:
   :undoc-members:

variables
===============
.. automodule:: variables
   :members:
   :undoc-members:

   .. autodata:: variables.variables

Decay String
===============
.. toctree:: DecayString

Event based analysis
====================
.. toctree:: event-based

MC matching
===========
.. toctree:: MCMatching

Advanced Tools
===============

flavorTagger
-------------

.. automodule:: flavorTagger
   :members:
   :undoc-members:

fei
---
.. automodule:: fei
   :members:
   :undoc-members:
   :imported-members:


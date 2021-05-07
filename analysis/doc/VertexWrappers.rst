.. _vertex:

Vertex-fitting convenience functions
====================================

.. hint:: A vertex fit will most likely change the kinematic properties of the decay head (mother) and,
          if the option ``daughtersUpdate`` (`kFit`) or ``updateAllDaughters`` (`treeFit`) is turned on,
          also of the daughter particles.
          To store the quantities prior to the vertex fit and be able to write them out to a ntuple for further analysis
          it is recommended to run `variablesToExtraInfo` before the vertex fit.

.. automodule:: vertex
   :members:
   :undoc-members:

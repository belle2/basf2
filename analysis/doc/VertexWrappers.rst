.. _vertex:

Vertex-fitting convenience functions
====================================

.. hint:: A vertex fit will most likely change the kinematic properties of the decay head (mother) and,
          if the option ``daughtersUpdate`` (`kFit`) or ``updateAllDaughters`` (`treeFit`) is turned on,
          also of the daughter particles.
          The variables of daughters before the vertex fit can be accessed with the meta-variable `originalDaughter`.
          For the variables of the mother particle, one can store the quantities in the extraInfo field by running
          `variablesToExtraInfo` before the vertex fit and then accessing them via the variable `extraInfo`.

.. automodule:: vertex
   :members:
   :undoc-members:

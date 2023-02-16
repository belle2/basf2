.. _vertex:

Vertex-fitting convenience functions
====================================

.. hint:: A vertex fit will most likely change the kinematic properties of the decay head (mother) and,
          if the option ``daughtersUpdate`` (`kFit`) or ``updateAllDaughters`` (`treeFit`) is turned on,
          also of the daughter particles.
          The variables of daughters before the vertex fit can be available with the meta-variable `originalDaughter`.
          For the variables of the mother particle, one can store the quantities in the extraInfo field by running the
          `variablesToExtraInfo` before the vertex fit so that one can get them with the `extraInfo`.

.. automodule:: vertex
   :members:
   :undoc-members:

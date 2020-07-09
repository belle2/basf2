Variables
=========

Technical documentation about the usage of variables is given in :ref:`analysis/doc/index-01-analysis:variables`.

As you can see, there are many, many variables in ``basf2``. To make your live easier, the
``VariableManager`` allows to

- Get lists of variables by group (e.g. all kinematic variables), without having to specify
  them directly
- Define alias, i.e. shorthand notations of otherwise very long variable names
- ...

.. literalinclude:: ../../analysis/examples/VariableManager/variableManager.py

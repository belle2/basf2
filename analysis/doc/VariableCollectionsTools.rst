.. _variablecollectionstools:

Operations with variable lists
==============================

It is possible to create new variable lists using meta-variables. 
For example, one can define list of kinematical variables in LAB frame and create another lists of kinematic variabels 
in CMS using ``useCMSFrame(variable)`` meta-variable:

.. code:: python

  # Replacement to Kinematics tool
  kinematics = ['px',
                'py',
                'pz',
                'pt',
                'p',
                'E']
  # Kinematic variables in CMS
  ckm_kinematics = wrap_list(kinematics,
                             "useCMSFrame(variable)",
                             "CMS")

Functions for list operations are stored below.

.. automodule:: variableCollectionsTools
   :members:

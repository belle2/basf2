.. _analysismodules:

Full list of analysis modules
=============================

The Belle II physics analysis software follows the logic of the general Belle II Analysis Software Framework (basf2) where independent modules solve relatively small problems, where practically all communication between modules happens in the form of data objects. Modules are executed in a linear fashion inside a module path, and can make objects available “downstream” by saving them in the DataStore.

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
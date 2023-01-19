.. _onlinebook_workflowmanagement_b2luigi_offlineanalysis:

Offline Analysis
================

Generally, a b2luigi task is written as a python class and contains :code:`requires()` and :code:`output()`, which respectively require other tasks as input and specify output files. For most tasks, :code:`run()` generates the output files from the input files. The offline-analysis might then look like this:

.. literalinclude:: offlineanalysis.py
   :language: python
   :linenos:
   :caption:

The :code:`MergeFiles` task employs :code:`b2luigi.basf2_helper.Basf2nTupleMergeTask` to merge the reconstructed nTuples for all skims specified in :code:`qqcontinuumskims.dat` for quark-antiquark continuum and :code:`bmesonsskims.dat` for B mesons.

.. literalinclude:: ../qqcontinuumskims.dat
   :language: python
   :linenos:
   :caption:
   
.. literalinclude:: ../bmesonsskims.dat
   :language: python
   :linenos:
   :caption:

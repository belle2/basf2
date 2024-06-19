.. _onlinebook_workflowmanegement_b2luigi_reconstruction:

Reconstruction on KEKcc Batch System
====================================

Let us now look at the reconstruction, which produces the reconstructed nTuples required by the :code:`MergeFiles` task.

.. literalinclude:: reconstruction.py
   :language: python
   :linenos:
   :caption:
   
The reconstruction task :code:`ReconstructBatch` employs :code:`b2luigi.basf2_helper.Basf2PathTask` to run a minimal steering script on the input mdst list produced by the :code:`BatchesToTextFile` task. The reconstruction tasks are the only tasks not marked as :code:`local` and will therefore be scheduled to the KEKcc batch system. The :code:`ReconstructionWrapper` task merges all reconstructed batches of a skim.
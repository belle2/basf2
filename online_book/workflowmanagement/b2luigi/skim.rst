.. _onlinebook_workflowmanagement_b2luigi_skim:

Skim using gbasf2
=================

Finally, let us look at the skim task whose output files make up the input mdst lists for the reconstruction.

.. literalinclude:: skim.py
   :language: python
   :linenos:
   :caption:
   
The :code:`BatchesToTextFile` task fills text files with lists of skim output mdst paths, where :code:`NumBatches` specifies the number of batches per skim. Instead of through text files, one could pass a batch directly to the reconstruction by merging the corresponding skim output files. However direct, this would require a significant amount of additional storage space. 

The :code:`SkimTask` task again employs :code:`b2luigi.basf2_helper.Basf2PathTask` to run a minimal steering script on the specified datasets and has a number of parameters specific to gbasf2, such as :code:`batch_system = "gbasf2"`. b2luigi will automatically setup a proxy, reschedule failed gbasf2 jobs a maximum of :code:`gbasf2_max_retries` times, download the skims and check for their completeness. Make sure that you are using the latest release of b2luigi, which may not be on the python package index.
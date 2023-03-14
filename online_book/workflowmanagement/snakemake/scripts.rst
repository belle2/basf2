.. _onlinebook_workflowmanagement_snakemake_scripts:

Separate Scripts for the Snakemake Workflow
===========================================

For completeness, we here provide the separate analysis scripts called by the snakemake workflow. Existing scripts can be implemented with the :code:`script:` directive in the *snakefile* with minimal adaptations. The input and output file paths, as well as the parameters for the corresponding rule can be retrieved in the scripts with :code:`snakemake.input`, :code:`snakemake.output` and :code:`snakemake.params`.

.. literalinclude:: offlineanalysis.py
   :language: python
   :linenos:
   :caption:
   
.. literalinclude:: reconstruction.py
   :language: python
   :linenos:
   :caption:

.. literalinclude:: batchToTxt.py
   :language: python
   :linenos:
   :caption:
   
When using our provided :code:`wrapper:` for submitting gbasf2 jobs to the grid, the steering file does not need any specific adaptations.

.. literalinclude:: skim.py
   :language: python
   :linenos:
   :caption:

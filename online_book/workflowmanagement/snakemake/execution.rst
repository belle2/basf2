.. _onlinebook_workflowmanagement_snakemake_execution:

Full Workflow Execution
=======================

We execute the full dependency tree of the workflow by calling :code:`snakemake --cores <ncores> -s snakefile`, where we point to the snakefile and :code:`<ncores>` specifies the maximum number of parallelly running tasks. Do not forget to setup basf2 beforehand, for the recommended release use :code:`b2setup $(b2help-releases)`.

To submit rules to the KEKcc batch system, we can specify :code:`snakemake --jobs <njobs> -s snakefile --cluster bsub`, where :code:`<njobs>` specifies the maximum number of parallelly running tasks. In our case, we only want to submit the reconstructions, therefore we mark all other rules as local in the snakefile:

.. literalinclude:: localrules
   :language: none
   
You can run snakemake workflows dryly with :code:`snakemake -npr -s snakefile`, with printout for shell commands and a reasoning why every rule is triggered.

Snakemake does not feature a dynamic directed acyclic graph (DAG). For dynamic workflow monitoring you may us the `Panoptes Server <https://github.com/panoptes-organization/panoptes>`_. To statically visualize the DAG for your workflow, call:

.. literalinclude:: visualize.sh
   :language: shell
   
Snakemake workflows can be executed remotely on the `Reproducible research data analysis platform <https://reanahub.io/>`_ for added functionalities.

.. _onlinebook_workflowmanagement_snakemake_snakefile:

Workflow Logic Code in the Snakefile
====================================

In snakemake all workflow logic is centralized in the so-called *snakefile*, similar to a make file. All analysis code is detached in separate scripts, with minimal adaptations to accommodate them in the workflow logic. Snakemake supports multiple programming languages; we will stick to Python.

Processing steps in a snakemake workflow are called *rules*, written in a simple Python-based language and typically consist of :code:`input:` files, :code:`output:` files and a directive to create the latter. Available directives are :code:`shell:` commands, python code in :code:`run:` or a :code:`script:` path. Let us examine the snakefile for our Belle II workflow.

.. literalinclude:: snakefile
   :language: python
   :linenos:
   :caption:
   
In a snakefile the topmost rule is the target rule, in our case :code:`OfflineAnalysis` which produces the histogram plots. Rules can have :code:`log:` and :code:`params:` directives for log file paths and additional parameters respectively. 

The output file directory structure is specified in the file paths and created automatically. To avoid repetition, we can use wildcards that are filled in automatically. For example the rule with output :code:`"data/projectName_{EventType}/reco.root"` will be called as many times as there are values for :code:`{EventType}` (here twice for its two values :code:`bmesons` and :code:`qqcontinuum`). 

To merge basf2 output files, we can simply employ :code:`hadd` or :code:`b2file_merge` in a :code:`shell:` directive. 

To submit jobs to the grid using gbasf2, we provide a public wrapper via :code:`git clone https://github.com/casschmitt/gbasf2_wrapper_for_snakemake.git`. Specify :code:`wrapper: "file:/path/to/gbasf2_wrapper_for_snakemake"` as a directive in the rules that you want to submit using gbasf2. It takes care of proxy setup, job submission, reschedules failed jobs and downloads finished job outputs. To make sure the proxy is only initialized once, please include a rule with :code:`setProxy = True` and require its output :code:`proxy_text_file` in the skim rules. After checking for download completeness, it returns a text file with the paths to all output files, which can be used in subsequent processing steps. Please note that gbasf2 does not support absolute paths for the sandbox files. Global gbasf2-specific parameters can be given in a :code:`configfile:`

.. literalinclude:: config.yaml
   :language: yaml
   :linenos:
   :caption:

To map gbasf2 input file paths to output directories, we here use a dictionary :code:`PathDictionary`, which is filled with paths from the provided text files:

.. literalinclude:: ../qqcontinuumskims.dat
   :language: none
   :linenos:
   :caption:
   
.. literalinclude:: ../bmesonsskims.dat
   :language: none
   :linenos:
   :caption:

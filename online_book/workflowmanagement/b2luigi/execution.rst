.. _onlinebook_workflowmanagement_b2luigi_execution:

Full Workflow Execution
=======================

We execute the dependency tree for a target task by calling :code:`b2luigi.process(Task(parameters=...),workers=<nworkers>)`. b2luigi will run a maximum number of :code:`<nworkers>` tasks in parallel, whenever possible.

It is best practice  to include a :code:`__main__` method in the scripts:

.. literalinclude:: main.py
   :language: python
   :linenos:
   :caption:

Calling :code:`python3 main.py --batch` on KEKcc will the trigger the full workflow execution. b2luigi will build the dependency tree for the :code:`Plot` task and execute only the required tasks for which no output files are existing in the given output directory.  Do not forget to adjust :code:`output_directory` and to setup basf2 beforehand, for the recommended release use :code:`b2setup $(b2help-releases)`. Remember that the reconstruction task is the only task not marked as :code:`local` and will therefore be submitted to the KEKcc batch system. 

You can run b2luigi workflows dryly with :code:`python3 main.py --dry-run` to check what tasks would be run.
    
Luigi features a dynamic directed acyclic graph, that can be viewed in the *Luigi Task Status*. To access it, start the luigi scheduler in a tmux process on KEKcc and specify the host and port in the workflow execution:

.. literalinclude:: visualize.txt
   :language: shell
   :linenos:

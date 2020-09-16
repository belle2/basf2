.. _onlinebook_batch:

Batch submission
================
.. sidebar:: Overview
   :class: overview

..    **Teaching**: 10 min

..    **Exercise**: 5 min

    **Prerequisites**:
        * A `KEKCC account <https://belle.kek.jp/secured2/secretary/registration/comp_system.html>`_

    **Questions**:

        * How to check batch queues for Belle II workgroup?
        * How to submit a job to a specific queue?
        * How to check the status of running jobs?
        * How to cancel a batch job?



A batch job is submitted from a work server at KEKCC, the job is scheduled by 
LSF (Platform Load Sharing Facility developed by IBM) which will dispatches 
and executes the job on a calculation server. It is important to select an 
appropriate queue for your jobs.
In this lession, we will go through some commands that often used in analysis.

Basic commands
--------------

.. rubric:: Displays information about batch queues

It is important to know which queues can be used and what is the workload of 
the queue.

To display the information about all batch queues:

.. code-block:: tcsh

   bqueues [-u <user name>|<user group>]

If no option is given, this returns the following information about all 
queues: queue name, queue priority, queue status, task statistics, and 
job state statistics.

This command also displays the current "Fairshare" values. Fairshare 
defines the priorities of jobs that are dispatched.

.. code-block:: tcsh

   bqueues -l [<queue_name>]

.. admonition:: Excercise
      :class: exercise stacked

      Check your priorities on queue s. 

.. admonition:: Hint
      :class: toggle xhint

      Try to specify queue name to get the relevant information.


.. rubric:: Submit a job

To submit a basf2 job to a batch queue

.. code-block:: tcsh

   bsub -q <queue name> "basf2 <your_working_script>"

.. note::
   Always test your script before submitting large scale of jobs to batch system.

.. rubric:: Display job status

To check the job status

.. code-block:: tcsh

   bjobs [-q <queue name>] [<job_ID>]

.. rubric:: Cancel a job

To cancel jobs

.. code-block:: tcsh

   bkill [<job_ID>]

.. note::

   Use ``0`` to kill all jobs. Use with caution.

In the case that the following situations occurred:

* When executed bkill command, there is a message "Job <JOBID> is being 
terminated"
* ``bjobs`` keeps showing the job that should have been terminated

.. code-block:: tcsh

   bkill -r <job_ID>


Optional
--------
In some case you might want to stop the submitted jobs and resume them later, 
for instance, scheduled maintenance of storage elements where the input data 
is located, or updating analysis global tags that used in your jobs.

.. rubric:: Suspend jobs

To suspend unfinished jobs

.. code-block:: tcsh

   bstop <job_ID>

.. note::

   Use -a to suspend all jobs.

.. rubric:: Resume jobs

   To resumes suspended jobs

.. code-block:: tcsh

      bresume <job_ID>



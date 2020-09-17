.. _onlinebook_batch:

Batch submission
================
.. sidebar:: Overview
   :class: overview

    **Prerequisites**:
        * A `KEKCC account <https://belle.kek.jp/secured2/secretary/registration/comp_system.html>`_

    **Questions**:

        * How to check batch queues for Belle II workgroup?
        * How to submit a job to a specific queue?
        * How to check the status of running jobs?
        * How to cancel a batch job?



A batch job is submitted from a work server at KEKCC, the job is scheduled by 
`LSF <https://www.ibm.com/support/knowledgecenter/en/SSWRJV_10.1.0/lsf_welcome/lsf_kc_using.html>`_ 
(Platform Load Sharing Facility developed by IBM) which will dispatches 
and executes the job on a calculation server. It is important to select an 
appropriate queue for your jobs.
In this lession, we will go through some commands that often used in analysis.

Basic commands
--------------

.. rubric:: Displays information about batch queues

It is important to know which queues can be used and what is the workload of 
the queue.

To display the information about all batch queues:

.. code-block:: bash

   bqueues [-u $USER]

If no option is given, this returns the following information about all 
queues: queue name, queue priority, queue status, task statistics, and 
job state statistics.

This command also displays the current "Fairshare" values. Fairshare 
defines the priorities of jobs that are dispatched.

.. code-block:: bash

   bqueues -l [<queue_name>]

Here the square brackets [...] indicate that the argument is optional 
and <...> indicates that the value should be filled in by you.

.. admonition:: Exercise
   :class: exercise stacked

   Check your priorities on queue s. 

.. admonition:: Solution
   :class: toggle solution

   .. code-block:: bash

      bqueues -l s [| grep $USER]

   Provide queue name after ``-l``, and combine with ``grep``
   command to get your information more quickly.
   If you never use batch queue before, it should be 0.333.


Every uses has the default value of 0.333 to start with.
The more jobs you submit, the lower your Fairshare is.


.. rubric:: Submit a job

With an example script as

.. code-block:: bash

   #!/usr/bin/bash
   echo "Hello world, this is script ${0}." >> batch_output.txt
   sleep 20
   echo "Finished!" >> batch_output.txt

To submit a job to queue s

.. code-block:: bash

   bsub -q s "bash example.sh"

and check the output

.. code-block:: bash

   $ cat batch_output.txt
   Hello world, this is script example.sh.
   Finished!

Use the same method you can submit python or basf2 scripts to bqueues!

.. code-block:: bash

   bsub -q <queue name> "basf2 <your_working_script>"

.. note::
   Always test your script before submitting large scale of jobs to batch system.


.. rubric:: Display job status

To check the job status

.. code-block:: bash

   bjobs [-q <queue name>] [<job_ID>]

.. admonition:: Exercise
   :class: exercise stacked

      Submit a basf2 job to queue l, and then check the status of your jobs.

.. admonition:: Solution
   :class: toggle solution

      Submission:

      .. code-block:: bash

         $ bsub -q l "basf2 one_of_example.py"
         Job <xxxxxxxx> is submitted to queue <l>.

      Check status:

      .. code-block:: bash

         $bjobs


.. rubric:: Cancel a job

To cancel jobs

.. code-block:: bash

   bkill [<job_ID>]

.. note::

   Use ``0`` to kill all jobs. Use this with caution.

Sometimes ``bjob`` will still show the job after we tried to terminate it. 
In this case we can use the ``-r`` option to force kill it. 
More information is given `here 
<https://www.ibm.com/support/knowledgecenter/en/SSWRJV_10.1.0/lsf_users_guide/job_kill_force.html>`_.

Optional
--------
In some case you might want to stop the submitted jobs and resume them later, 
for instance, scheduled maintenance of storage elements where the input data 
is located, or updating analysis global tags that used in your jobs.

.. rubric:: Suspend jobs

To suspend unfinished jobs

.. code-block:: bash

   bstop <job_ID>

.. note::

   Use -a to suspend all jobs.

.. rubric:: Resume jobs

   To resumes suspended jobs

.. code-block:: bash

      bresume <job_ID>



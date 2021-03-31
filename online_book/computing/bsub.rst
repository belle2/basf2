.. _onlinebook_bsub:

Batch submission
================

.. sidebar:: Overview
   :class: overview

    **Teaching**: 10 min

    **Exercises**: 10 min

    **Prerequisites**:

        * A `KEKCC account <https://belle.kek.jp/secured2/secretary/registration/comp_system.html>`_

    **Questions**:

        * How to check batch queues for Belle II workgroup?
        * How to submit a job to a specific queue?
        * How to check the status of running jobs?
        * How to cancel a batch job?

    **Objectives**:

        * Submit computation intensive scripts to worker servers

When a batch job is submitted from a work server at KEKCC, the job is scheduled by
`LSF <https://www.ibm.com/support/knowledgecenter/en/SSWRJV_10.1.0/lsf_welcome/lsf_kc_using.html>`_
(Platform Load Sharing Facility developed by IBM) which dispatches
and executes the job on a calculation server. It is important to select an
appropriate queue for your jobs.

In this lesson, we will go through some commands that are often used in analysis.

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

.. code-block:: bash

   $ bqueues -u $USER
   QUEUE_NAME      PRIO STATUS          MAX JL/U JL/P JL/H NJOBS  PEND   RUN  SUSP
   s               120  Open:Active    3000  600    -    - 30885 28774  2111     0
   l               100  Open:Active       - 1000    -    - 39959 29395 10564     0
   h               100  Open:Active    1500  300    -    -  1719  1416   303     0
   p               100  Open:Active    1500  300    -    -   577     0   577     0
   sx              100  Open:Active       -  200    -    -  2185  1986   199     0
   lx              100  Open:Active       -  200    -    -     2     0     2     0
   hx              100  Open:Active     300   60    -    -     0     0     0     0
   px              100  Open:Active     300  100    -    -     0     0     0     0
   P1              100  Open:Active       -    -    -    -     0     0     0     0
   Pmpi            100  Open:Active       -    -    -    -     0     0     0     0
   b_b             100  Open:Active       - 1000    -    -     0     0     0     0
   cmb_p           100  Open:Active       -  300    -    -     0     0     0     0
   cmb_px          100  Open:Active       -  100    -   10     0     0     0     0
   a               100  Open:Active       -    4    -    -    11     3     8     0
   dc_generic      100  Open:Active       -    -    -    -     0     0     0     0

Different queues have different settings. For analysis you can use ``s``,
``l``, or ``h``. For short jobs with running time under 3 hours, the queue
``s`` is preferable. For jobs with execution time more than 3 hours,
you might want to use the queue ``l`` which gives jobs up to 24 hours of
running time.
More information about LSF queues can be found
`here <https://kekcc.kek.jp/service/kekcc/support/en/12/>`__.

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
   If you have never used the batch queue before, it should be 0.333.


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

Use the same method, you can submit Python or basf2 scripts to bqueues!

.. code-block:: bash

   bsub -q <queue name> "basf2 <your_working_script>"

.. note::
   Always test your script before submitting large scale jobs to batch system.


.. rubric:: Display job status

To check the job status

.. code-block:: bash

   bjobs [-q <queue name>] [<job_ID>]

.. admonition:: Exercise
   :class: exercise stacked

      Submit a ``basf2`` job to queue ``l``, and then check the status of your jobs.

.. admonition:: Hint
   :class: xhint stacked toggle

   A simple ``basf2`` job could be the following:

   .. code-block:: python

      # Print all variables known to the variable manager
      from variables import printVars
      printVars()

.. admonition:: Solution
   :class: toggle solution

      Submission:

      .. code-block:: bash

         $ bsub -q l "basf2 one_of_example.py"
         Job <xxxxxxxx> is submitted to queue <l>.

      To check the status, use one of the following:

      ``bjobs -q l <xxxxxxxx>``, ``bjobs <xxxxxxxx>``, or just ``bjobs`` alone.

.. rubric:: Cancel a job

To cancel jobs

.. code-block:: bash

   bkill [<job_ID>]

.. note::

   Use ``0`` to kill all jobs. Use this with caution.

Sometimes ``bjobs`` will still show the job after we tried to terminate it.
In this case we can use the ``-r`` option to kill it by force.
More information is given `here
<https://www.ibm.com/support/knowledgecenter/en/SSWRJV_10.1.0/lsf_users_guide/job_kill_force.html>`__.

Optional
--------
In some scenarios you might want to stop the submitted jobs and resume them
later. For instance this might be due to scheduled maintenance of storage
elements where the input data is located or the updating of analysis global tags
that used in your jobs.

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

.. admonition:: Key points
   :class: key-points

   * Submit a script to the short queue with ``bsub -q s "bash myscript.sh"``
   * Check job queues with ``bequeues``
   * Kill jobs with ``bkill <job id>``
   * **Always test your scripts before large scale submissions!**

.. include:: ../lesson_footer.rstinclude

.. topic:: Author of this lesson

   Chia-Ling Hsu

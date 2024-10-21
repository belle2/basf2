.. _onlinebook_workflowmanagement_checkpointing:

Checkpointing for machine learning
==================================
   
.. sidebar:: Overview
    :class: overview

    **Teaching**: 10 minutes

    **Prerequisites**:

    * a python environment

    **Questions**:

    * What is Checkpointing?
    * How can I use Checkpointing in my machine learning workflow?

    **Objectives**:

    * Understand the concept of Checkpointing
    * Learn how to use Checkpointing in your machine learning workflow

Checkpointing describes the process of storing the current state of the program to be able to continue the work at a different time and/or place.
Having checkpoints of workflows is helpful in case of failures or crashes and can save time and resource when not everything has to be re-run.

Checkpointing an arbitrary workflow however can be almost endlessly complex.
However, very repetitive workflows can be checkpointed with little effort.
Common examples in the context of high energy physics are event loops and machine learning workflows.
For event loops, Checkpointing is not so interesting, since this work is trivially parallelizable:
the workload can be split in independent batches to minimize the risk of losing work.

Training machine learning models however can not be parallelized, since one epoch depends on the epoch before.
Additionally, the training time can be very long, which makes crashes more severe and time constraints on host sites a big issue.
Therefore, Checkpointing can be very useful.

A tool to help with Checkpointing can be found on `github <https://github.com/JonasEppelt/Checkpointer>`_.

To learn how to use it, please refer to the `documentation <https://github.com/JonasEppelt/Checkpointer/blob/main/README.md>`_ and consult the `examples <https://github.com/JonasEppelt/Checkpointer/tree/main/examples>`_.

.. include:: ../lesson_footer.rstinclude

.. topic:: Author of this lesson

    Jonas Eppelt

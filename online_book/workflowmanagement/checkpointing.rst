.. _onlinebook_workflowmanagement_checkpointing:

Checkpointing for Machine Learning
=======
   
.. sidebar:: Overview
    :class: overview

    **Teaching**: 10 minutes

    **Prerequisites**:

    * a python enviroment

    **Questions**:

    * What is Checkpointing?
    * How can I use Checkpointing in my Machine Learning workflow?

    **Objectives**:

    * Understand the concept of Checkpointing
    * Learn how to use Checkpointing in your Machine Learning workflow

Checkpointing describes the process of storing the current state of the program to be able to continue the work at a different time and/or place.
Haveing checkpoints of workflows is helpfull in case of failures or crashes and can save time and resource when not everything has to be re-run.

Checkpointing an arbitrary workflow however can be almost endlessly complex.
However, very repetetive workflows can be checkpointed with little effort.
Common examples in the context of High Energy Physics are event loops and machine learning workflows.
For event loops, checkpointing is not so interresting, since this work is trivially parallelizable:
The workload can be split in independent batches to minimize the risk of loosing work.

Training Machine Learning models however can not be parallelized, since one epoch depends on the epoch before.
Additionally trainings can run very long and are therefore prone to crashes or can run into time constraints on their host site.
Therefore, checkpointing can be very useful.

A tool to help with checkpointing can be found on the DESY gitlab: [cehckpointer](https://gitlab.desy.de/jonas.eppelt/checkpointer).

To learn how to use it, please refer to the [documentation](https://gitlab.desy.de/jonas.eppelt/checkpointer/-/blob/master/README.md) and consult the [examples](https://gitlab.desy.de/jonas.eppelt/checkpointer/-/tree/master/examples).

.. include:: ../lesson_footer.rstinclude

.. topic:: Author of this lesson

    Jonas Eppelt

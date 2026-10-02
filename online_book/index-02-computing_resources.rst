.. _onlinebook_computing_resources:

Resources
=========

The aim of this page is to:

1. Make you aware of the computing resources you have access to
2. Provide you with links with more information and helpful tutorials

.. _onlinebook_batch:

Batch Submission
----------------

Batch submission refers to the process of submitting large numbers of computing jobs to a distributed
computing system rather than running them interactively on a local machine. It allows jobs to be run
and monitored in parallel across the system. There are two batch schedulers that manage
the distribution of the jobs: HTCondor and LSF.

HTCondor
~~~~~~~~

HTCondor is an open-source batch scheduler built for high-throughput computing. Batch submission using the DESY-hosted
NAF servers (see :ref:`onlinebook_naf`) so if you want to submit batch jobs there, you will need to use HTCondor. 

For more information and tutorials on how to use HTCondor, you can look at the `CERN HTCondor User Guide <https://batchdocs.web.cern.ch/index.html>`_.
It also covers some basic concepts of batch computing in the context of HTCondor. 

LSF
~~~

LSF stands for Load Sharing Facility and is developed by IBM. It is used by the KEK computing system for batch
submission. Therefore, if you want to submit batch jobs on KEKCC, you will need to use LSF. 

For a brief overview of the relevant commands, you can refer to the
`IBM Quick Reference Guide <https://www.ibm.com/docs/en/spectrum-lsf/10.1.0?topic=started-quick-reference>`_. More details on the capabilities
of LSF can also be found from this page.  

.. _onlinebook_grid:

Grid
----

The Belle II distributed computing system, also known as the Grid, is a form of computing where a "virtual super computer"
is composed of many loosely networked computers. To date, 75 computing sites and 32 storage elements contribute to the
distributed computing resources, managed by central services hosted at KEK and BNL. This allows us to execute 20K jobs
in a geographically distrbuted environment. 

Copies of all the data and MC samples for Belle II are stored on the Grid. If you want to know how to use the Grid for your
analysis, you can look at the extensive `gbasf2 documentation <gbasf2.belle2.org>`_.

.. _onlinebook_naf:

NAF
---

The National Analysis Facility (NAF) is a multi-purpose batch-cluster optimised for high throughput running HTCondor as a
scheduling system. The NAF at DESY complements the DESY Grid resources, offers computing and storage services for all
German scientists working on ATLAS, CMS and ILC as well as the entire Belle II collaboration. 

To apply for a NAF account, please go to the registration link on the `Internal Links <https://www.belle2.org/internal_links/>`_
page. 

Documentation for NAF, including a short walk-through, can be found on the `NAF Documentation <https://docs.desy.de/naf/>`_ page.
It is also recommended to have a look at the
`Best Practices and Examples <https://gitlab.desy.de/belle2/computing/computing-facilities/desy-analysis-facility/-/wikis/home/Best-Practices-and-Examples>`_ page.

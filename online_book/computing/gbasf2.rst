.. _onlinebook_gbasf2:

gbasf2
======

.. sidebar:: Overview
    :class: overview

    **Teaching**: 20 min

    **Exercises**: 10-20 min

    **Prerequisites**:

        * Go through `computing getting started <https://confluence.desy.de/display/BI/Computing+GettingStarted>`_ for getting access to the grid services.
        * An ``ssh`` client and the :ref:`onlinebook_ssh` lesson.
        * A system with SL6 or CentOS 7.
        * Grid certificate installed in ``~/.globus`` and on the web browser.

    **Questions**:

        * What is gbasf2?
        * Where to search files on the grid?
        * How I monitor my jobs?
        * What to do if I need help?

    **Objectives**:

        * Install gbasf2 and set the environment for using grid tools.
        * Search datasets stored on the grid.
        * Submit jobs using gbasf2.
        * Monitor your jobs and take action if there are issues.
        * Check the documentation and ask for help if necessary.
        * Download the output for offline analysis.


Gbasf2 (grid + basf2) is an extension of basf2, from your desktop to the grid.
One advantage is the same steering files used with basf2 work with gbasf2. 

The usual workflow is
first developing a Basf2 steering file,
testing it locally and finally,
submit the jobs to the grid with the same steering file.

Before getting started, make sure you understand the following.
The GRID is NOT a local computing system like KEKCC.
Once you submit jobs, they will be assigned to computing systems around the world.
If your job is problematic, it will be distributed to the world and all sites will be affected.

.. warning::

    You must carefully check your jobs with a local computing system, e.g. KEKCC, before you submit jobs to GRID.


Installing gbasf2
-----------------

Unfortunately, at this moment the basf2 and gbasf2 environments are not compatible. This means gbasf2 requires
a fresh ssh session (without sourcing ``b2setup``).

Open a terminal and create a directory to store your gbasf2 installation. Inside, let's download the
installation script and execute it specifying the installation type with ``-V Belle-KEK``. In short:

.. code-block:: bash

        mkdir gbasf2 && cd gbasf2
        wget -N http://dirac.cc.kek.jp/dirac/dirac-install.py
        python dirac-install.py -V Belle-KEK

Check that the execution finished without errors. If you see error messages, please
check the `gbasf2 troubleshooting <https://confluence.desy.de/display/BI/GBasf2+Troubleshooting>`_.


Locating datasets on the grid
-----------------------------

Files are stored around the world in the different storage elements.
A logical file name (LFN) is the unique identifier of a file in the Belle II grid in the form of a unix-like file path
(starting always with /belle):

.. code-block:: bash

        /belle/data_type/some_more_directories/file_name

Examples of LFNs are

.. code-block:: bash

        # A mdst dataset of data from exp 10
        /belle/Data/proc/release-04-02-02/DB00000938/proc11/prod00013368/e0010/4S/r03774/mdst

        # A MC sample of charged B mesons
        /belle/MC/release-04-00-03/DB00000757/MC13a/prod00009435/s00/e1003/4S/r00000/charged/mdst

A replica catalog resolves the LFN, and provides the information of where to find the file(s).
Then, you only need to provide the LFN(s) of the datasets which are relevant for your analysis.

The Dataset Searcher is a web application to find datasets on the grid.
Go to the `DIRAC webportal <https://dirac.cc.kek.jp:8443/DIRAC/>`_ and then open
Menu -> BelleDIRACApps -> Dataset Searcher.

Another way to interact with the dataset searcher is using the command ``gb2_ds_search``.

.. admonition:: Exercise
     :class: exercise stacked

     Set your gbasf2 environment and try to get the LFN of MC uubar samples from MC13a, with beam energy of 4S
     and background level BGx1.

.. admonition:: Hint
     :class: toggle xhint stacked

     Use ``--help`` and ``--usage`` to get all the available options

.. admonition:: Solution
     :class: toggle solution

     ``gb2_ds_search dataset --data_type mc --campaign MC13a --beam_energy 4S --mc_event uubar --bkg_level BGx1``




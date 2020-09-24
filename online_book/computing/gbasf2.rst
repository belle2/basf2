.. _onlinebook_gbasf2:

Gbasf2
======

.. sidebar:: Overview
    :class: overview

    **Teaching**: 45 min

    **Exercises**: 20-30 min

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


Gbasf2 is an extension of basf2, from your desktop to the grid (see :ref:`onlinebook_computing_system`).
Data and MC samples are distributed in many storage sites around the world, and the gbasf2 tools allow you to access and
analyze such distributed data sets.

The same steering files used with basf2 work with gbasf2. 
The usual workflow is:

* First developing a basf2 steering file.
* Testing it locally.
* Locate your input files on the grid.
* Submit the jobs to the grid with the same steering file.

.. admonition:: Key points
    :class: key-points

    Before getting started, make sure you understand the following:

    * The GRID is NOT a local computing system like KEKCC.
    * Once you submit jobs, they will be assigned to computing systems around the world.
    * If your job is problematic, it will be distributed to the world and all sites will be affected.

Installing gbasf2
-----------------

Unfortunately, at this moment the basf2 and gbasf2 environments are not compatible. This means gbasf2 requires
a fresh ssh session (without sourcing ``b2setup``).

Open a terminal and create a directory to store your gbasf2 installation. Inside, let's download the
installation script:

.. code-block:: bash

        mkdir gbasf2 && cd gbasf2
        wget -N http://dirac.cc.kek.jp/dirac/dirac-install.py
        python dirac-install.py -V Belle-KEK

Execute the installation script specifying the installation type with ``-V Belle-KEK``:

.. code-block:: bash

        python dirac-install.py -V Belle-KEK

Check that the execution finished without errors. If you see error messages,
check the `gbasf2 troubleshooting <https://confluence.desy.de/display/BI/GBasf2+Troubleshooting>`_.

Proceed to the post-installation configuration:

.. code-block:: bash

        source bashrc && dirac-proxy-init -x
        dirac-configure defaults-Belle-KEK.cfg

Setting your gbasf2 environment
-------------------------------

Once the above installation is done, you only need to execute two commands every time that you open a new terminal:

.. code-block:: bash

        source ~/gbasf2/BelleDIRAC/gbasf2/tools/setup
        gb2_proxy_init -g belle

It will ask for your certificate password before generating your credentials. Once created, your proxy will be valid
for 24 hours. You just need to execute ``gb2_proxy_init`` again.


Locating datasets on the grid
-----------------------------

The most common task as user of the grid is the submission of jobs with input files

* From the official Belle MC campaigns.

* From the official data reprocessing and skims.

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
Menu -> BelleDIRACApps -> Dataset Searcher. You have the option of searching between data or MC, samples
with beam background (BGx1) or without (BGx0), and several fields to refine your search. Play with all the available
options and get familiar with them.

The ``MC Event types`` box show by default the generic samples available (charged, mixed, uubar, etc.).
If you want to search
signal samples, you need to specify the `signal event type <https://confluence.desy.de/display/BI/Signal+EventType>`_.

.. admonition:: Exercise
     :class: exercise stacked

     Open the Dataset Searcher and obtain the LFN of of the MC13a
     signal sample ``B0 -> [J/psi -> e+e-][Ks -> pi+ pi-]``, with beam background (BGx1) in the simulation.

.. admonition:: Hint
     :class: toggle xhint stacked

     Search the `signal event type <https://confluence.desy.de/display/BI/Signal+EventType>`_ of the decay.

.. admonition:: Solution
     :class: toggle solution

     /belle/MC/release-04-00-03/DB00000757/MC13a/prod00012867/s00/e1003/4S/r00000/1111540100/mdst


Another way to interact with the dataset searcher is using the command line tool ``gb2_ds_search``.

.. admonition:: Exercise
     :class: exercise stacked

     Set your gbasf2 environment and try to get the LFNs of MC uubar samples from MC13a, with beam energy of 4S
     and background level BGx1.

.. admonition:: Hint
     :class: toggle xhint stacked

     Use ``--help`` and ``--usage`` to get all the available options

.. admonition:: Solution
     :class: toggle solution

     ``gb2_ds_search dataset --data_type mc --campaign MC13a --beam_energy 4S --mc_event uubar --bkg_level BGx1``



Submit your first job to the Grid
---------------------------------

As mentioned before, gbasf2 uses exactly the same steering files of basf2 to submit jobs to the grid. The basic usage is

.. code-block:: bash

        gbasf2 <your_script.py> -p <project_name> -s <available_basf2_release>

where ``project_name`` is a name assigned by you, and ``available_basf2_release`` is the available Basf2 software
version to use.

.. warning::

    Do not use special characters in the project names ($, #, %, /, etc.),
    it could create problems with file names in some sites and in the databases.




.. warning::

    You must carefully check your jobs with a local computing system, e.g. KEKCC, before you submit jobs to GRID.

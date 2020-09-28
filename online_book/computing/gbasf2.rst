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


Gbasf2 is an extension of basf2, from your desktop to the grid.
Data and MC samples are distributed in many storage sites around the world, and the gbasf2 tools allow you to access and
analyze them.

.. seealso:: :ref:`onlinebook_computing_system`

The same steering files used with basf2 work with gbasf2, and the usual workflow is:

* First developing a basf2 steering file.
* Testing it locally.
* Locate your input files.
* Submit jobs to the grid with the same steering file.
* Download the output to perform the offline analysis (plots, fits, etc.)


.. warning::

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

Check that the execution finished without errors.

.. tip::

    If you see error messages,
    a `gbasf2 troubleshooting <https://confluence.desy.de/display/BI/GBasf2+Troubleshooting>`_ is available.

Proceed to the post-installation configuration:

.. code-block:: bash

        source bashrc && dirac-proxy-init -x
        dirac-configure defaults-Belle-KEK.cfg

Setting your gbasf2 environment
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Once the above installation is done, you only need to execute two commands every time that you open a new terminal:

.. code-block:: bash

        source ~/gbasf2/BelleDIRAC/gbasf2/tools/setup
        gb2_proxy_init -g belle

It will ask for your certificate password before generating your credentials. Once created, your proxy will be valid
for 24 hours. You just need to execute ``gb2_proxy_init -g belle`` again if your credentials expire.

.. seealso:: https://confluence.desy.de/display/BI/Computing+GBasf2


Locating datasets on the grid
-----------------------------

The most common task as user of the grid is the submission of jobs with input files

* From the official Belle MC campaigns.

* From the official data reprocessing and skims.

Files are stored around the world in the different storage elements.
Fortunately, as user you don't have to worry about the physical location.
A file catalog keeps the record of where the files are located, and you just need to provide a logical identifier
of the interesting samples for your analysis.

Datasets and Datablocks
^^^^^^^^^^^^^^^^^^^^^^^

A **logical file name** (LFN) is the unique identifier of a file in the Belle II grid in the form of a unix-like file path
(starting always with /belle):

.. code-block:: bash

        /belle/data_type/some_more_directories/dataset/datablock/file

A replica catalog resolves the LFN, and provides the information of where to find the files.
Then, you only need to provide the LFN(s) relevant for your analysis, without dealing with the physical location of the
samples.

Files are classified inside datasets.
Examples of LFNs for datasets are:

.. code-block:: bash

        # A mdst dataset of data from exp 10
        /belle/Data/proc/release-04-02-02/DB00000938/proc11/prod00013368/e0010/4S/r03774/mdst

        # A MC sample of charged B mesons
        /belle/MC/release-04-00-03/DB00000757/MC13a/prod00009435/s00/e1003/4S/r00000/charged/mdst

By design, a directory on the grid can only contain 1000 files at most. For this reason, the concept of datablock
is introduced. Each dataset is subdivided by directories with name ``subXX``, where the last two digits are sequential
(``sub00, sub01, ...``).

.. admonition:: Key points
    :class: key-points

    * By design, each datablock contains a maximum of 1000 files.
    * If a dataset contains more than 1000 files, at least it will be subdivided in two datablocks.

.. tip::

    The command-line tool for listing the content of a directory on the grid is ``gb2_ds_list``
    (it is equivalent to ``ls`` on your local system).

.. admonition:: Exercise
     :class: exercise stacked

     Use ``gb2_ds_list`` to see how datablocks contain the dataset

     ``/belle/MC/release-04-00-03/DB00000757/MC13a/prod00012386/s00/e1003/4S/r00000/eeee/mdst``

.. admonition:: Hint
     :class: toggle xhint stacked

     Remember to set your gbasf2 environment first, otherwise the tool will not be found.

.. admonition:: Solution
     :class: toggle solution

     ``gb2_ds_list /belle/MC/release-04-00-03/DB00000757/MC13a/prod00012386/s00/e1003/4S/r00000/eeee/mdst``
     will show you that the dataset contains 3 datablocks.


.. tip::

    Sometimes, in the documentation (such as Confluence pages) we refer to the **logical path name** (LPN)
    of datasets and datablocks, while for files we keep LFN. In practice, LFN and LPN are the same thing.

The Dataset Searcher
^^^^^^^^^^^^^^^^^^^^

The Dataset Searcher is a web application to find datasets on the grid.
Go to the `DIRAC webportal <https://dirac.cc.kek.jp:8443/DIRAC/>`_ and then open
Menu (the icon at the left-bottom) -> BelleDIRACApps -> Dataset Searcher.

You have the option of searching between data or MC, samples
with beam background (BGx1) or without (BGx0), and several fields to refine your search. Play with all the available
options and get familiar with them.

.. figure:: DatasetSearcher.png
    :align: center
    :width: 600px
    :alt: The dataset searcher

    The Dataset Searcher at the DIRAC web portal.

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

.. admonition:: Another hint
     :class: toggle xhint stacked

     The event type is ``1111540100``.

.. admonition:: Solution
     :class: toggle solution

     /belle/MC/release-04-00-03/DB00000757/MC13a/prod00012867/s00/e1003/4S/r00000/1111540100/mdst


.. tip::

    You can download a list of LFNs from the Dataset Searcher using the button "Download txt file" at the bottom.


Another way to interact with the dataset searcher is using the command line tool ``gb2_ds_search``.

.. admonition:: Exercise
     :class: exercise stacked

     Set your gbasf2 environment and try to get the LFNs of MC uubar samples from MC13a, with beam energy of 4S
     and background level BGx1 using ``gb2_ds_search``.

.. admonition:: Hint
     :class: toggle xhint stacked

     Use ``--help`` and ``--usage`` to get all the available options

.. admonition:: Solution
     :class: toggle solution

     ``gb2_ds_search dataset --data_type mc --campaign MC13a --beam_energy 4S --mc_event uubar --bkg_level BGx1``



Submit your first jobs to the Grid
----------------------------------

As mentioned before, gbasf2 uses exactly the same steering files of basf2 to submit jobs to the grid. The basic usage is

.. code-block:: bash

        gbasf2 <your_steering_file.py> -p <project_name> -s <available_basf2_release>

where ``project_name`` is a name assigned by you, and ``available_basf2_release`` is the available basf2 software
version to use.

.. note::

    The maximum length for a project name is 32 characters.


.. warning::

    Do not use special characters in the project names ($, #, %, /, etc.),
    it could create problems with file names in some sites and in the databases.


Once located the dataset to use for your analysis, you can specify the LFN of the **datablock** to use as input with
with the flag ``-i``.

.. note::

    While the Dataset Searcher provides the LFN for datasets, gbasf2 uses for now datablocks as input. You need to append
    ``sub00, sub01, ...`` to the LFNs provided by the Dataset Searcher (this will be fixed in the near future, sorry for
    the inconvenience).

Everything clear? Ok, let's submit your first jobs.

.. warning::

    Remember: you must carefully check your jobs with a local computing system,
    e.g. KEKCC, before you submit jobs to GRID.


Let's use the steering file located at
``~michmx/public/tutorial2020/Reconstruct_Bd2JpsiKS_template.py`` on KEKCC (take a look at what contains).
If we are interested in running over a generic uubar sample, then the LFN of one datablock is
``/belle/MC/release-04-00-03/DB00000757/MC13a/prod00009436/s00/e1003/4S/r00000/uubar/mdst/sub00`` (you obtained it in a
previous exercise, remember?).

With all this information,
let's submit the gbasf2 jobs:

.. code-block:: bash

    gbasf2 -p gb2Tutorial_Bd2JpsiKs -s light-2002-ichep \
           -i /belle/MC/release-04-00-03/DB00000757/MC13a/prod00009436/s00/e1003/4S/r00000/uubar/mdst/sub00 \
           ~michmx/public/tutorial2020/Reconstruct_Bd2JpsiKS_template.py

    TODO: Show the output of the command (after the downtime of the grid finishes)

.. admonition:: Question
     :class: exercise stacked

     What is the name of the project and the basf2 release in the example above?

.. admonition:: Solution
     :class: toggle solution

     The name of the project is ``gb2Tutorial_BdJpsiKs`` and the light release is ``light-2002-ichep``.


.. tip::

    You can check which basf2 releases are available for running jobs on the grid using ``gb2_check_release``.


.. admonition:: Exercise
     :class: exercise stacked

     Submit a gbasf2 job with an steering file built by you in previous chapters of the book.

     * Search an interesting datablock to use as input.
     * Prepare your steering file.
     * Submit using gbasf2.

.. admonition:: Hint
     :class: toggle xhint stacked

     "Interesting datablock" means, if you are reconstructing B+ mesons for example, you may want to run
     over a ``charged`` datablock to get as many signal events as possible,
     or to see how the background of your signal looks.

.. admonition:: Additional hint
     :class: toggle xhint stacked

     Use the flag ``--usage`` to see the available options.

.. admonition:: Solution
     :class: toggle solution

     gbasf2 -i <dataset> -s release-05-00-00 -p <your project name> <your steering file>


.. admonition:: Key points
    :class: key-points

    * A gbasf2 project can be submitted **per datablock**, NOT per dataset.
        * We will fix this in coming gbasf2 releases.

    * Inside the project, gbasf2 will produce file-by-file jobs.

    * The number of output files in the project will be the number of files in the input datablock.



Submit jobs with multiple LFNs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you want to submit a project with several datablocks, prepare a list of LFNs on a file and provide it to gbasf2 using
``--input_dslist``.

.. tip::

    A quick way of appending ``sub00`` to a list of LFNs obtained from the Dataset Searcher is using ``sed``:

    .. code-block:: bash

        sed -i 's/mdst/mdst\/sub00/g' listOfLFNs.list



Monitoring jobs
---------------


Downloading the output
----------------------








.. topic:: Author of this lesson

    Michel Villanueva

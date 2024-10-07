.. _onlinebook_gbasf2:

Gbasf2
======

.. sidebar:: Overview
    :class: overview

    **Teaching**: 45 min

    **Exercises**: 20-30 min

    **Prerequisites**:

    * See `Computing getting started <https://xwiki.desy.de/xwiki/rest/p/c804c>`_.
    * Access to KEKCC or NAF (recommended), or to a system with CentOS 7 or EL9 and CVMFS mounted.
    * A valid `grid certificate <https://xwiki.desy.de/xwiki/rest/p/d9328>`_ installed in ``~/.globus`` and in a web browser.
    * A working basf2 steering script (see the :ref:`onlinebook_first_steering_file` lesson)

    **Questions**:

    * What is gbasf2?
    * How do I find input files on the grid?
    * How do I monitor my jobs?
    * What do I do if I need help?

    **Objectives**:

    * Set up the gbasf2 environment.
    * Search for datasets stored on the grid.
    * Submit jobs using gbasf2.
    * Monitor jobs and take action if there are issues.
    * Check the documentation and ask for help if necessary.
    * Download the output for offline analysis.

Gbasf2 is the command-line client for submitting grid-based basf2 jobs.
Belle II data and MC samples are distributed in many storage sites around the world. Gbasf2 and a set of grid-based user tools (gb2 tools) allow you to access and
analyze them.

The same basf2 steering files are used when running on the grid. The usual workflow is:

* Develop a basf2 steering file.
* Test it locally (don't skip this step!).
* Locate your input files on the grid.
* Submit jobs to the grid with the same basf2 steering file.
* Download the output to perform offline analysis (plots, fits, etc.).

.. warning::

    Before getting started, make sure you understand the following:

    * The GRID is NOT a local computing system like KEKCC.
    * Once you submit jobs, they will be assigned to computing systems around the world.
    * If your job is problematic, it will be distributed to the world and many sites will be affected.
    * Remember, always test your jobs locally before submitting to the grid!


Go to `computing getting started <https://xwiki.desy.de/xwiki/rest/p/c804c>`_
and verify that you have the prerequisites. You need:

* Access to KEKCC or NAF (recommended), or to a system with CentOS 7 and CVMFS mounted.
* A valid grid certificate issued within a year and `installed <https://xwiki.desy.de/xwiki/rest/p/c804c/#H2.Installyourcertificate>`_ in ``~/.globus`` in ``.pem`` format.
* Belle Virtual Organization (VO) membership registered or renewed within a year. You can check your status at
  `VOMS server <https://voms.cc.kek.jp:8443/voms/belle/>`_.
* Registration in `DIRAC <https://xwiki.desy.de/xwiki/rest/p/c804c/#H7.RegisterwithDIRAC>`_.

.. note::

    It is required that you join the `comp users forum <https://lists.belle2.org/sympa/info/comp-users-forum>`_,
    where you can ask for help and receive announcements on releases and system issues. You can also find more
    details on the official `gbasf2 documentation page <https://gbasf2.belle2.org/>`_


Setting up gbasf2 via cvmfs
---------------------------

Since the DIRAC user interface relies on some middleware components, this limits the operating environments in which
gbasf2 can function. At this moment, only CentOS 7 is supported (it works on other EL9-based operating systems like AlmaLinux 9 but please be aware it is not fully validated).

Also, unfortunately at this moment the basf2 and gbasf2 environments are not compatible. This means gbasf2 requires
a fresh ssh session (without sourcing ``b2setup``).

.. note::

    Be sure that the userkey.pem has the ``rw`` permissions only for the owner and no permission to the others.
    You should see ``-rw-------`` with ``ls -l ~/.globus/userkey.pem``. Otherwise, use

    .. code-block:: bash

        chmod 600 ~/.globus/userkey.pem

If your computing system has access to cvmfs (e.g. at KEKCC), the simplest way to use gbasf2 is via a central installation. The following command sets all the necessary environment variables and initializes a grid proxy for you (you will be asked to enter your credentials for this).

.. code-block:: bash

    source /cvmfs/belle.kek.jp/grid/gbasf2/pro/bashrc

It will request your certificate passphrase. If the command finishes without errors and you see information related to your certificate, similar to that below, your proxy has been successfully set:

.. code-block:: bash

    Proxy generated:
    subject      : /C=JP/O=KEK/OU=CRC/CN=USERNAME
    issuer       : /C=JP/O=KEK/OU=CRC/CN=USERNAME
    identity     : /C=JP/O=KEK/OU=CRC/CN=USERNAME
    timeleft     : 23:53:58
    DIRAC group  : belle
    rfc          : True
    path         : /tmp/x100up_u0001
    username     : youruser
    properties   : NormalUser
    VOMS         : True
    VOMS fqan    : ['/belle']

    Succeed with return value:
    0

That's it! You are ready to run grid jobs!

A proxy is a short-term credential that verifies your identity, allowing you to perform operations on the grid. By default your proxy is valid for 24h. If it expires, you need to execute ``gb2_proxy_init -g belle`` again if you have already setup gbasf2 in the same shell (terminal).

.. note::

    You can also pass arguments when setting up gbasf2, e.g. if you need to setup a grid proxy with permissions beyond the default user (with option -g).



Installing gbasf2
-----------------

The pre-installed gbasf2 on CVMFS is the recommended version (see Setup Gbasf2). However, gbasf2 local installation is available. Please be aware this is not fully tested and not fully supported. If you wish to install gbasf2 locally, follow the instructions on the `gbasf2 documentation <https://gbasf2.belle2.org/gbasf2install.html>`_.

.. seealso:: https://gbasf2.belle2.org/


Locating datasets on the grid
-----------------------------

The most common task as user of the grid is the submission of jobs with input files

* From the official Belle MC campaigns.

* From the official data reprocessing and skims.

Files are stored around the world in the different storage elements.
Fortunately, as users, you don't have to worry about the physical location.
A file catalog keeps the record of where the files are located, and you just need to provide a logical identifier
of the samples of interest for your analysis.

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

        # An mDST dataset of data from exp 18
        /belle/Data/release-06-00-08/DB00002392/proc13/prod00029531/e0018/4S/r00000/mdst

        # A uDST dataset of data from exp 18
        /belle/Data/release-06-01-11/DB00002058/proc13/prod00029781/e0018/4S/r01394/hadron/14120601/udst

        # An MC sample of charged B mesons
        /belle/MC/release-06-01-08/DB00002649/MC15rd_b/prod00027830/s00/e0026/4S/r01950/charged/mdst

By design, a directory on the grid can only contain 1000 files at most. For this reason, the concept of datablock
is introduced. Each dataset is subdivided by directories with name ``subXX``, where the last two digits are sequential
(``sub00, sub01, ...``).

.. admonition:: Key points
    :class: key-points

    * By design, each datablock contains a maximum of 1000 files.
    * If a dataset contains more than 1000 files, it will be subdivided into at least two datablocks.

The command-line tool for listing the content of a directory on the grid is ``gb2_ds_list``
(it is equivalent to ``ls`` on your local system). You can use it to see how many datablock(s) is/are contained
in each dataset.

.. tip::

    All the gbasf2 command-line tools (sometimes called gb2 tools) have the flags ``--help``
    and ``--usage`` to see all the available options.

.. admonition:: Exercise
     :class: exercise stacked

     Use ``gb2_ds_list`` to see how many datablock(s) is/are contained in the skimmed dataset

     ``/belle/Data/release-06-01-11/DB00002058/proc13/prod00029825/e0018/4S_offres/r02559/hadron/14120601/udst``

.. admonition:: Hint
     :class: toggle xhint stacked

     Remember to set your gbasf2 environment first, otherwise the tool will not be found.

.. admonition:: Solution
     :class: toggle solution

     Executing ``gb2_ds_list /belle/Data/release-06-01-11/DB00002058/proc13/prod00029825/e0018/4S_offres/r02559/hadron/14120601/udst``
     will show you that the dataset contains 1 datablock, ``sub00``.

.. note::

    Sometimes, in the documentation (such as in the XWiki pages) we refer to the **logical path name** (LPN)
    of datasets and datablocks, while for files we use LFN. In practice, LFN and LPN are the same thing.

The Dataset Searcher
^^^^^^^^^^^^^^^^^^^^

The Dataset Searcher is a web application to find datasets on the grid.
Go to the `DIRAC webportal <https://dirac.cc.kek.jp:8443/DIRAC/>`_ and then open
Menu (the icon at the left-bottom) -> BelleDIRAC Apps -> Dataset Searcher.

You have the option of searching between data or MC, samples
with beam background (BGx1) or without (BGx0), and several other fields to refine your search. Play with all the
available options and get familiar with them.

.. figure:: DatasetSearcher.png
    :align: center
    :width: 600px
    :alt: The dataset searcher

    The Dataset Searcher at the DIRAC web portal.

.. note::

    Accessing the DIRAC web portal requires that your user certificate is installed in your web browser. See
    `computing getting started <https://xwiki.desy.de/xwiki/rest/p/c804c>`_ for details.


The ``MC Event types`` box show, by default, the generic samples available (charged, mixed, uubar, etc.).
If you want to search
signal samples, you need to specify the `signal event type <https://xwiki.desy.de/xwiki/rest/p/3a506>`_.
Also, if you want to search uDST skim samples - which are what we will be using in the examples - you can find them at
:ref:`skim/doc/04-experts:Skim Registry` section in the basf2 software documentation.

.. note::

    Here, and in some of the exercises/examples to follow, we will be using the data-level format known as **uDST**.
    uDSTs (short for **user Data Summary Table**) is a format type that results from performing analysis skims on an
    input dataset (usually of mDST format) that reduces the size of the input dataset to a more manageable size by
    applying certain selection cuts. By doing this, the uDST contains a select amount of events from the input dataset
    that can be useful for a certain type of analysis.

.. note::

    For further information about uDST skims, including the skim code and the decay name associated with that code,
    see the basf2
    `skim registry <https://gitlab.desy.de/belle2/software/basf2/-/tree/main/skim/scripts/skim/registry.py>`_.

.. admonition:: Exercise
     :class: exercise stacked

     Open the Dataset Searcher and obtain the first LFN you see with an ``MC Event type`` of ``mixed`` from the
     uDST skim sample for the decay mode ``B0 -> [D- -> K_S0 pi-]pi+``, with beam background (BGx1) in the simulation.

.. admonition:: Hint
     :class: toggle xhint stacked

     Search the basf2
     `skim registry <https://gitlab.desy.de/belle2/software/basf2/-/tree/main/skim/scripts/skim/registry.py>`_ and look
     at the skim codes and names.

.. admonition:: Another hint
     :class: toggle xhint stacked

     The skim code is ``14120601``.

.. admonition:: Solution
     :class: toggle solution

     The first LFN shown with an ``MC Event type`` of ``mixed`` for skim code ``14120601`` is
     ``/belle/MC/release-04-02-00/DB00000898/SkimM13ax1/prod00013006/e1003/4S/r00000/mixed/14120601/udst``.


.. tip::

    You can download a list of LFNs from the Dataset Searcher using the button "Download txt file" at the bottom.


Another way to interact with the dataset searcher is using the command line tool ``gb2_ds_search``.

.. admonition:: Exercise
     :class: exercise stacked

     Set your gbasf2 environment and try to get the LFNs of MC uubar samples using the same skim code from the
     decay mode above (``14120601``), and using campaign ``MC15rd_b`` along with beam energy of 4S and background
     level BGx1 using ``gb2_ds_search``.

.. admonition:: Hint
     :class: toggle xhint stacked

     Use ``--help`` and ``--usage`` to get all the available options

.. admonition:: Solution
     :class: toggle solution

     The execution and result from the command line are as follows:

     .. code-block:: bash

         gb2_ds_search dataset --data_type mc --skim_decay 14120601 --campaign MC15rd_b --beam_energy 4S --mc_event uubar --bkg_level BGx1

         Matching datasets found:
	 /belle/MC/release-06-01-11/DB00002058/MC15rd_b/prod00030843/s00/e0007/4S/r00000/uubar/14120601/udst
	 /belle/MC/release-06-01-11/DB00002058/MC15rd_b/prod00030871/s00/e0008/4S/r00000/uubar/14120601/udst
	 /belle/MC/release-06-01-11/DB00002058/MC15rd_b/prod00030899/s00/e0010/4S/r00000/uubar/14120601/udst
	 /belle/MC/release-06-01-11/DB00002058/MC15rd_b/prod00030927/s00/e0012/4S/r00000/uubar/14120601/udst
	 /belle/MC/release-06-01-11/DB00002058/MC15rd_b/prod00031011/s00/e0017/4S/r00000/uubar/14120601/udst
	 /belle/MC/release-06-01-11/DB00002058/MC15rd_b/prod00031095/s00/e0022/4S/r00000/uubar/14120601/udst
	 /belle/MC/release-06-01-11/DB00002058/MC15rd_b/prod00030955/s00/e0014/4S/r00000/uubar/14120601/udst
	 /belle/MC/release-06-01-11/DB00002058/MC15rd_b/prod00031151/s00/e0024/4S/r00000/uubar/14120601/udst
	 /belle/MC/release-06-01-11/DB00002058/MC15rd_b/prod00031123/s00/e0022/4S/r00000/uubar/14120601/udst
	 /belle/MC/release-06-01-11/DB00002058/MC15rd_b/prod00032932/s00/e0020/4S/r00000/uubar/14120601/udst
	 /belle/MC/release-06-01-11/DB00002058/MC15rd_b/prod00032922/s00/e0018/4S/r00000/uubar/14120601/udst
	 /belle/MC/release-06-01-11/DB00002058/MC15rd_b/prod00032908/s00/e0016/4S/r00000/uubar/14120601/udst
	 /belle/MC/release-06-01-11/DB00002058/MC15rd_b/prod00032954/s00/e0026/4S/r00000/uubar/14120601/udst
	 /belle/MC/release-06-01-11/DB00002058/MC15rd_b/prod00032969/s00/e0026/4S/r00000/uubar/14120601/udst


Collections
----------------------------------

Rather than expect every analyst to use the dataset searcher to discover the data and MC samples useful for analysis,
the data production team prepares ``collections`` of LFNs. This provides a common, immutable, intuitive resource,
which can help to avoid errors related to missing or using incorrect samples for an analysis. You can discover 
collections in the same way as for LFNs.

To discover the collection you need, you should first check `Data main page <https://xwiki.desy.de/xwiki/rest/p/ed439>`_ 
and `MC main page <https://xwiki.desy.de/xwiki/rest/p/70b0a>`_, which provide details for the official collections.

If you want to explore the available collections, you can also use the dataset searcher tool. You've already 
used the ``gb2_ds_search`` tool to search for datasets. Now use the ``collection`` key word to list the available data collections.

.. admonition:: Exercise
     :class: exercise stacked

     Set your gbasf2 environment and find the available data collections for Moriond2023 using ``gb2_ds_search``.

.. admonition:: Hint
     :class: toggle xhint stacked

     Use ``--help`` and ``--usage`` to get all the available options

.. admonition:: Solution
     :class: toggle solution

     The execution and result from the command line are as follows:

     .. code-block:: bash

         gb2_ds_search collection --list_all_collections /belle/collection/Data/Moriond2023*

You can also do things like list the datasets within a collection with ``gb2_ds_search collection --list_datasets <COLLECTION>`` 
and get the metadata for the collection with ``gb2_ds_search collection --get_metadata <COLLECTION>``.

.. admonition:: Key points
    :class: key-points

    * Collections contain a complete sample of a particular type, e.g. the Moriond 2023 data sample

    * You should always refer to the data production pages to find the recommended collections.



Submit your first jobs to the Grid
----------------------------------

As mentioned before, gbasf2 uses exactly the same steering files of basf2 to submit jobs to the grid. The basic usage is

.. code-block:: bash

        gbasf2 <your_steering_file.py> -p <project_name> -s <available_basf2_release>

where ``project_name`` is a name assigned by you, and ``available_basf2_release`` is the available basf2 software
version to use.

.. note::

    The maximum length for a project name is 32 characters and the project name must be unique and cannot ever be reused, even if the project is deleted. One way to avoid problems with these restrictions is to use a short label and the date (eg. YYMMDD), for example ``B2JpsiKs_4Sdata_240119``.


.. warning::

    Do not use special characters in the project names ($, #, %, /, etc.),
    it could create problems with file names in some sites and in the databases
    (we allow only ``[^a-zA-Z0-9+-_]``).


Once you located the dataset to use for your analysis, you can specify the LPN of the **dataset** to use as input
with the flag ``-i``.

.. note::

    The Dataset Searcher provides the LPN for datasets which gbasf2 can use as input when submitting jobs.
    Prior to the latest releases of BelleDIRAC, there was a need to append ``sub00, sub01, ...`` to the LPNs provided
    by the Dataset Searcher, but datablocks are now automatically appended to the dataset LPN when jobs are submitted.
    You can, if you wish to use only one datablock, append to the end of the dataset LPN the datablock of your
    choosing, but this is no longer strictly required.

Everything clear? Ok, let's submit your first jobs.

.. warning::

    Remember: you **must** carefully check your jobs with a local computing system,
    e.g. KEKCC, before you submit jobs to GRID.


Let's use the steering file located at
``~jbennett/public/gbasf2Tutorial/Reconstruct_Bd2JpsiKS_template.py`` on KEKCC (take a look at what it contains).
If we are interested in running over a generic uubar sample, then the LFN of one datablock is
``/belle/MC/release-06-01-10/DB00002752/MC15rd_b/prod00029583/s00/e0018/4S/r00870/uubar/mdst/sub00``.

With all this information,
we are ready to submit the gbasf2 jobs, right?

STOP! Did you test the script locally first? Make sure it works before submitting to the grid!

Now that you've checked to make sure it works, let's submit a gbasf2 project:

.. code-block:: bash

    gbasf2 -p gb2Tutorial_Bd2JpsiKs_240101 -s light-2311-nebelung \
           -i /belle/MC/release-06-01-10/DB00002752/MC15rd_b/prod00029583/s00/e0018/4S/r00870/uubar/mdst \
           ~michmx/public/tutorial2020/Reconstruct_Bd2JpsiKS_template.py

A project summary and a confirmation prompt will be displayed after executing gbasf2

.. code-block:: bash

    ************************************************
    *************** Project summary ****************
    ** Project name: gb2Tutorial_Bd2JpsiKs_240119
    ** Dataset path: /belle/user/jbennett/gb2Tutorial_Bd2JpsiKs_240119
    ** Steering file: /home/belle2/jbennett/public/gbasf2Tutorial/Reconstruct_Bd2JpsiKS_template.py
    ** Job owner: jbennett @ belle (16:29:58)
    ** Preferred site / SE: None / None
    ** Input files for first job: LFN:/belle/MC/release-06-01-10/DB00002752/MC15rd_b/prod00029583/s00/e0018/4S/r00870/uubar/mdst/sub00/mdst_000001_prod00029583_task17869000001.root
    ** Number of input files: 1
    ** Number of jobs: 1
    ** Processed data (MB): 840
    ** Processed events: 127516 events
    ** Estimated CPU time per job: 2126 min
    ************************************************
    Are you sure to submit the project?
    Please enter Y or N:

After verifying that everything is correct, you can confirm the submission.

.. admonition:: Question
     :class: exercise stacked

     What is the the basf2 release in the example above?

.. admonition:: Solution
     :class: toggle solution

     The basf2 light release is ``light-2311-nebelung``.


.. tip::

    You can check which basf2 releases are available for running jobs on the grid using ``gb2_check_release``.

.. admonition:: Key points
    :class: key-points

    * A gbasf2 project can be submitted **per dataset** or **per datablock**.

      * If submitted **per dataset**, all datablocks within the specified dataset will be resolved.

      * The most useful way to submit an analysis project is using collections! Just use the collection path as the input argument. We won't do that here, since the collections are quite large and you shouldn't use them until you are ready to run a real analysis.

    * Inside the project, gbasf2 will produce jobs file-by-file according to the input.

    * The number of output files in the project will be equal to the number of files in the input.


.. admonition:: Exercise
     :class: exercise stacked

     Submit a gbasf2 job with an steering file built by you in previous chapters of the book, for analyzing
     a datablock of MC15rd_b, MC Event Types ``charged`` at center-of-mass energy ``4S``.
     Use ``light-2311-nebelung`` of basf2.

     Remember:

     * Prepare your steering file.
     * Search the input dataset.
     * Submit using gbasf2.

.. admonition:: Hint
     :class: toggle xhint stacked

     Use the Dataset Searcher to locate MC15rd_b datasets of MC Event Types ``charged``.

.. admonition:: Additional hint
     :class: toggle xhint stacked

     The input dataset may be obtained using

     .. code-block:: bash

        gb2_ds_search dataset --data_type mc --campaign MC15rd_b --beam_energy 4S --mc_event charged

     Next, choose one of the datasets listed after executing the command above and use that dataset with the command
     ``gb2_ds_list <dataset_LPN>`` to see what datablock(s) are available in your chose dataset.
     Recall that, if you wish to run over one specific datablock from a dataset, you can add ``sub00, sub01, ...``
     at the end of the dataset LPN.

.. admonition:: Solution
     :class: toggle solution

     .. code-block:: bash

        gbasf2 -i /belle/MC/release-06-01-10/DB00002752/MC15rd_b/prod00029588/s00/e0018/4S/r00092/charged/mdst
        -s light-2311-nebelung -p myFirstProject_240101 <your steering file>


Submit jobs with multiple LFNs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

As we have already stated, with the newest releases of BelleDIRAC, there is no longer a need to append the datablock
``(sub00, sub01, ...)`` to the end of the dataset LPN. If you want (or need) to submit a project with several datablocks, 
you can prepare a list of LFNs on a file and provide it to gbasf2 using ``--input_dslist``.

Monitoring jobs
---------------

There are two ways to monitor your jobs on the grid: command-line tools and the DIRAC web portal.

Monitoring in the terminal
^^^^^^^^^^^^^^^^^^^^^^^^^^

For the command-line tools, you can use ``gb2_project_summary``
to have an overview of your project (The flag ``-p`` will specify the project name):

.. code-block:: bash

    gb2_project_summary -p gb2Tutorial_Bd2JpsiKs_240119

           Project               Owner     Status    Done   Fail   Run   Wait   Submission Time(UTC)   Duration
    ==============================================================================================================
    gb2Tutorial_Bd2JpsiKs_240119   jbennett   Running   0      0      1     0      2024-01-20 02:14:57    00:01:30


.. tip::

    If no project name is specified, the tool will display information of your projects in the last month.


The gb2 tool ``gb2_job_status`` lists all the jobs running in a project, including the status and minor status:

.. code-block:: bash

    gb2_job_status -p gb2Tutorial_Bd2JpsiKs_240119

    5 jobs are selected.

     Job id     Status         MinorStatus        ApplicationStatus      Site
    =============================================================================
    387165066   Running   Application             Running             OSG.BNL.us

    --- Summary of Selected Jobs ---
    Completed:0  Deleted:0  Done:0  Failed:0  Killed:0  Running:1  Stalled:0  Waiting:0


Monitoring using the web portal
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The second way is looking at the job monitor in the `DIRAC web portal <https://dirac.cc.kek.jp:8443/DIRAC/>`_.

* Open the portal, click on the logo at the bottom-left and go to Applications/Job Monitor.
* You have to click on 'Submit' to display the information.

You should see something like this:

.. figure:: JobMonitor.png
    :align: center
    :width: 600px
    :alt: The job monitor

    The Job Monitor at the DIRAC web portal.

.. tip::

    The Job Monitor includes many tools and features to track and manage your jobs, including a statistics panel
    (pie icon at the left-top). Get familiar with them.


.. admonition:: Exercise
     :class: exercise stacked

     Monitor the jobs that you have submitted in the previous exercise. Wait until they finish successfully.

.. admonition:: Hint
     :class: toggle xhint stacked

     Do you see failed jobs? Go to the last section "Dealing with issues".

.. admonition:: Solution
     :class: toggle solution

     Use the DIRAC web portal and open the Job Monitor. Jobs in green are in 'Done' status, while the failed ones are in red.



Downloading the output
----------------------

If all your jobs finished successfully (have status 'Done'), then you can download the output.
The output files will be located below your user space (``/belle/user/<username>/<project_name>``).
You can check the output using ``gb2_ds_list <project_name>``:

.. code-block:: bash

    gb2_ds_list gb2Tutorial_Bd2JpsiKs_240119

    /belle/user/jbennett/gb2Tutorial_Bd2JpsiKs_240119/sub00

.. tip::

    To see the size of your output and its location, you can use the flags ``-l`` and ``-lg``.

To actually download the files, use ``gb2_ds_get``:

.. code-block:: bash

    gb2_ds_get gb2Tutorial_Bd2JpsiKs_240119

    Download 1 files from SE
    Trying to download davs://dcbldoor.sdcc.bnl.gov:443/pnfs/sdcc.bnl.gov/data/bellediskdata/TMP/belle/user/jbennett/gb2Tutorial_Bd2JpsiKs240119/sub00/Bd2KpsiKs_00000_job387165066_00.root to /localhome/jvbennet/gb2Tutorial_Bd2JpsiKs240119/sub00/Bd2KpsiKs_00000_job387165066_00.root

    Successfully downloaded files:
    /belle/user/jbennett/gb2Tutorial_Bd2JpsiKs240119/sub00/Bd2KpsiKs_00000_job387165066_00.root in /localhome/jvbennet/gb2Tutorial_Bd2JpsiKs240119/sub00


    Failed files:

.. tip::

    You can enable download in multiple streams using the ``--new`` option of ``gb2_ds_get``, which will speed up the transfer. At some point it will become the default behavior of ``gb2_ds_get``. 

.. tip::

    Keep in mind: as far as you have a gbasf2 installation, you can submit jobs or download files from any machine.

.. admonition:: Exercise
     :class: exercise stacked

     Download the output of your jobs submitted in a previous exercise. Verify that they are readable using ROOT.

.. admonition:: Hint
     :class: toggle xhint stacked

     First check that all your jobs finished successfully. Issues? Go to the next section.

.. admonition:: Solution
     :class: toggle solution

     Just use ``gb2_ds_get <your project name>`` (Easy, right?).


Dealing with issues
-------------------

Sometimes, things do not go well. A few jobs can fail because a large list of reasons, like

* A timeout in the transfer of a file between sites.
* A central service not available for a short period of time.
* An issue in the site hosting the job.
* etc.

Some of my jobs failed
^^^^^^^^^^^^^^^^^^^^^^

If you find that **some** of your jobs failed, most probably there was a temporal issue with your job or the site.
You need to reschedule these jobs by yourself.

You can use ``gb2_job_reschedule -p <project name>``:

.. code-block::

    gb2_job_reschedule --usage

    Resubmit failed jobs or projects.
    Only jobs which have fatal status (Failed, Killed, Stalled) are affected.
    Exact same sandbox and parameters are reused. Thus you may need to submit different job if they are wrong.

    By default, select only your jobs in current group.
    Please switch group and user name by options.
    All user's jobs are specified by '-u all'.

    Examples:

    % gb2_job_reschedule -j 723428,723429
    % gb2_job_reschedule -p project1 -u user

Or you can use the job monitor in the DIRAC web portal, selecting the failed jobs and clicking the **'Reschedule'**
button.

All my jobs failed
^^^^^^^^^^^^^^^^^^

If **all** your jobs failed, most probably something is wrong with the steering file or the gbasf2 arguments
(Did you test your steering file locally before submitting the jobs?).

A useful way to track which was the problem is (if possible) downloading the output sandbox. It contains the logs
related to your job.

.. figure:: getSandbox.png
    :align: center
    :width: 600px
    :alt: The job monitor

    How to download the output sandbox from the Job Monitor.


It is also possible to retrieve the log files directly from the command line using ``gb2_job_output``:

.. code-block:: bash

    gb2_job_output -j 387165066

    download output sandbox below ./log/JOBID
    1 jobs are selected.
    Please wait...
                               Result for jobs: ['387165066']
    =====================================================================================
    Downloaded: "Job output sandbox retrieved in /home/jbennett/log/387165066"

.. admonition:: Exercise
     :class: exercise stacked

     Download the output sandbox of one of your jobs. Check what is inside.

.. admonition:: Hint
     :class: toggle xhint stacked

     One of the logs inside may look very familiar.

.. admonition:: Solution
     :class: toggle solution

     The file ``basf2helper.py.log`` contains the actual output of your basf2 steering file executed on the grid site.


Where to go for help?
---------------------

The `comp users forum <https://lists.belle2.org/sympa/info/comp-users-forum>`_ is the main channel of communication
related to issues with the grid. Feel free to ask every time that you need help.

The most useful place to find additional instructions and information about using gbasf2 is `gbasf2.belle2.org <https://gbasf2.belle2.org>`_.

Additionally, some pages at XWiki are prepared with additional information:

* `Gbasf2 mainpage <https://xwiki.desy.de/xwiki/rest/p/78b3b>`_
* `Gbasf2 FAQ <https://xwiki.desy.de/xwiki/rest/p/f1a37>`_ and `troubleshooting <https://xwiki.desy.de/xwiki/rest/p/35008>`_
* `Computing glossary <https://xwiki.desy.de/xwiki/rest/p/d6530>`_

Take a look at the `gbasf2 tutorials <https://xwiki.desy.de/xwiki/rest/p/a21dd>`_ (they contain some advanced topics not covered here).

You can also ask in `questions.belle2.org <https://questions.belle2.org/questions>`_.

.. include:: ../lesson_footer.rstinclude

.. rubric:: Author of this lesson

Michel Villanueva, Justin Guilliams, Jake Bennett

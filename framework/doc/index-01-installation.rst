.. highlight:: shell

Installation and Setup
======================

This is a short version of the detailed `Installation Manual`_ in Confluence.

The Belle2 Software is meant to work an any recent 64 bit Linux system but it is only
tested and provided in binary form for a few select distributions

.. include:: supported-distributions.rst-fragment

If you run on one of these distributions the most convenient way to obtain the
Belle2 Software is to use it via CVMFS_ which is readily available on KEKCC and
many HEP specific software resources.  It can also easily be installed on your
local machine following the `CVMFS Client Quick Start`_ guide.

If you want to install the Belle II Software without cvmfs please have a look at
the following documents, otherwise if you have cvmfs available please continue with :ref:`cvmfs-setup`

.. toctree:: 
   :maxdepth: 1

   installation-tools
   installation-local
   installation-full

.. _cvmfs-setup:

Setup Of the Belle II Software
------------------------------

If CVMS is available on your machine you only have to setup
the :ref:`belle2-tools` by running ::

  $ source /cvmfs/belle.cern.ch/$distribution/tools/setup_belle2

where you replace ``$distribution`` with one of the short names for supported
distributions from the list at at the top of this page.

.. hint:: There are some :ref:`extra environment variables
   <belle2-tools-setup>` which can be set to customize the belle2 setup

.. note:: If you use the software without CVMFS please change the path to
  where you :ref:`installed the Belle II Software tools <belle2-tools-installation>`

.. warning:: There is a custom setup script on KEKCC. If you work on KEKCC
  please source ``/sw/belle2/tools/setup_belle2`` instead.

Now the only thing left to do is to setup which version of the Belle II Software
you want to use ::

  $ setuprel release-01-00-00

After that all the :ref:`command-line-tools` will be setup correctly and ready
to use.

.. hint:: to get a list of the available releases run ``get_release.sh``

.. warning:: this has to be done in every shell you plan on using the Belle II Software


Physics Analysis Setup
......................

If you want to develop your analysis you can setup your own analysis project with ::

  $ newana analysis_name release_version

where you should replace the ``analysis_name`` with a meaningful name for your
analysis. This will be the directory name for your project as well as the name
of the git repository on the server. ``release_version`` should be replaced
with the release version your analysis will be based on. After this you can ::

  $ cd $analysis_name
  $ setupana

to setup your analysis project. You can add your own basf2 `Module` to this
analysis by running ::

  $ newmod ModuleName

where ``ModuleName`` is the name of the module you want to create. The command
will ask you a few questions that should be more or less self-explanatory. The
requested information includes your name, module parameters, input and output
objects, methods, and descriptions for doxygen comments. If unsure you can
usually just hit enter. The ``newmod`` command will create a skeleton header
and source file of your module and include them in the files known to git.

To compile your code simply type ::

  $ scons

in your analysis working directory,

An advantage of having the analysis code in git is that you can check it out at
any other location and continue your work there. The git repository takes care
of synchronizing the multiple local version of the code. To get the code of an
existing analysis with a certain name type ::

  $ getana <analysis name> 
  
Again, changes can be submitted to the git repository with git commit followed
by  git push. To get the changes made in a different local version and
committed to the central repository to your current local analysis working
directory, use the command  ::

  $ git pull --rebase

Development Setup
.................

If you plan on developing code you should consider checking out the development
version locally instead of using a pre compiled release::

  $ newrel development

This will obtain the latest version from git. Once this is done you can setup
this version using ::

  $ cd development
  $ setuprel

And you can compile the code with  ::

  $ scons -D

(the ``-D`` makes sure that you can also run the command in a sub directory)

.. _CVMFS: https://cernvm.cern.ch/portal/filesystem
.. _CVMFS Client Quick Start: https://cernvm.cern.ch/portal/filesystem/quickstart
.. _Installation Manual: https://confluence.desy.de/x/gFwHAg

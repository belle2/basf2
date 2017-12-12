.. highlight:: shell

Installation and Setup
======================

This is a short version of the detailed `Installation Manual`_ in Confluence.

The Belle2 Software is meant to work an any recent 64 bit Linux system but it is only
tested and provided in binary form for a few select distributions

* Scientific Linux 6 or CentOS 6 (``sl6``)
* Enterprise Linux 7 or CentOS 7 (``el7``)
* Ubuntu 16.04 (``ubuntu1604``)

.. hint:: to find which distribution you are running on you can use
  ``lsb_release -a``

If you run on one of these distributions the most convenient way to obtain the
Belle2 Software is to use it via CVMFS_ which is readily available on KEKCC and
many HEP specific software resources.  It can also easily be installed on your
local machine following the `CVMFS Client Quick Start`_ guide.

Once CVMFS is installed you should be able to access
:file:`/cvmfs/belle.cern.ch` on your machine. Once this is done you only have
to execute :file:`/cvmfs/belle.cern.ch/{${distribution}}/tools/prepare_belle2.sh` 
once to check that all required dependencies are installed. In this path
``${distribution}}`` is the correct label for your Linux distribution from the
list above.

.. note:: Running ``ls /cvmfs`` or opening :file:`/cvmfs` in a graphical
  file browser might not show ``belle.cern.ch`` as the directories in there are
  only created on access so try to ``ls /cvmfs/belle.cern.ch``

If you run a different distribution please refer to the instructions for the
"Full Local Installation" in the `Installation Manual`_


Setup
-----

To setup the Belle 2 software you need to setup the belle 2 tools ::

  $ source /cvmfs/belle.cern.ch/${distribution}/tools/setup_belle2


.. note:: There is a custom setup script on KEKCC. If you work on KEKCC please
  source ``/sw/belle2/tools/setup_belle2`` instead.

followed by a setup of the release you want to use ::

  $ setuprel release-01-00-00

.. hint:: to get a list of the available releases run ``get_release.sh``

.. warning:: this has to be done in every shell you plan on using the Belle 2 Software


Physics Analysis Setup
----------------------

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
-----------------

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


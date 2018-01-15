.. _belle2-tools:

Belle II Software Tools
=======================

The Belle II Software Tools is a collection of script to prepare your
environment for the execution of the Belle2 software.

.. _belle2-tools-installation:

Installation
------------

In case you don't have a centrally provided Belle 2 Software you need to
install the tools locally.  Once you have `uploaded your ssh key
<Git/Stash Introduction>`_ to https://stash.desy.de can obtain the
software tools from the central Belle 2 code repository with the following
command::

    $ git clone ssh://git@stash.desy.de:7999/b2/tools.git

.. note:: we strongly recommend to setup connection over ssh but if you cannot
    do so you can also obtain the tools using ::

        $ git clone https://username@stash.desy.de/scm/b2/tools.git

    where you need to replace ``username`` with your DESY username


After first time installation of the tools you need to run ::

    $ tools/prepare_belle2.sh

to make sure that all software requirements for installing the Belle 2 Software
are met on your Machine.

.. note:: This will require root permissions


.. _belle2-tools-setup:

Setup
-----

Once the tools are installed you need to setup the Belle 2 environment by
sourcing ``tools/setup_belle2``::

    $ source tools/setup_belle2

.. warning:: this has to be done in every shell you plan on using the Belle 2 Software

The behaviour of the tools can be customized by setting the following
environment variables before sourcing ``tools/setup_belle2``:

.. envvar:: BELLE2_USER

   This variable allows you to set the username for any communication to the
   servers. By default it will be set to the local username.

   This variable is helpful if your local username is different to your DESY username.

.. envvar:: BELLE2_GIT_ACCESS

   This cariable can be set to either ``http`` or ``ssh`` to specify the
   protocol to be used for git commands. Please have a look at `Git/Stash
   Introduction`_ for details on the meainings.

   ``ssh``
      This will use ssh connection to the DESY git server. This is the
      preferred method but requires that you upload your ssh key to
      https://stash.desy.de and that TCP port 7999 is not blocked for outgoing
      connections by your local firewall.

   ``http``
      Use https to access the DESY git servers. This works in all cases but
      will require frequent entering of your desy password.

   If this variable is not set it will default to ``ssh`` if a file named
   :file:`~/.ssh/id_rsa` or :file:`~/.ssh/id_dsa` exists and to http if no such
   files exist.

.. envvar:: BELLE2_NO_TOOLS_CHECK

   If set to a non-empty value sourcing the tools will not try to check if the
   tools version is up to date. This option is useful for laptops without
   permanent internet connection.

   .. warning:: If you set this on your machine please check regularely that
      the tools are up to date by running ``git pull`` in the tools directory.

.. envvar:: VO_BELLE2_SW_DIR

   This should point to the parent directory of the tools directory and
   indicates where the tools and installed releases are to be found.

.. envvar:: BELLE2_EXTERNALS_TOPDIR

   Where to look for the external software. This only needs to be set if you
   installed the software externals in a different directory. The default is
   :file:`{$VO_BELLE2_SW_DIR}/externals`


Provided Scripts
----------------

The Belle II Software Tools provide a number of scripts common to all software
versions to setup and use the Belle II Software.

For users
+++++++++

.. describe:: getana

  ::

      Usage: getana directory [username]

  This command checks out the analysis code from the given repository name in
  git.  It also prepares the build system.

  The optional second argument can be used to specify an svn user name if it
  differs from the local user name.

.. describe:: get_externals.sh

  ::

      Usage: get_externals.sh [version [system]]

  This command installs the given version of the externals in the directory given
  by the environment variable :envvar:`BELLE2_EXTERNALS_TOPDIR`. If the operating
  system is specified it tries to install the corresponding precompiled binary
  version otherwise it will attempt to compile them from source.

  If no version is given it lists the available externals versions.

.. describe:: get_release.sh

  ::

      Usage: get_release.sh [version [system]]

  This command installs the given release or build version of basf2 in the
  directory :file:`{$VO_BELLE2_TOPDIR}/releases`. If the operating system is
  specified it tries to install the corresponding precompiled binary version,
  otherwise it will try to compile from source.

  If no version is given it lists the available versions.

.. describe:: newana

  ::

    Usage: newana directory release

  This command creates a local directory with the given name for the development
  of analysis code.  It also prepares the build system and adds the analysis
  directory to git.

  The second argument specifies the central release on which the analysis should
  be based.

.. describe:: setupext

   ::

     Usage: setupext [externals_version]

   This command sets up the Belle2 externals to be used without any specific release
   of the Belle II software. It's useful if you just want to enable the software
   included in the Belle2 externals like an updated ROOT or git version. Without an
   argument it will setup the latest version it can find, otherwise it will setup
   the specified version

.. describe:: setuprel

   ::

      Usage: setuprel [release]

   This command sets up the environment for a local and/or central release
   of the Belle II software.

   - Local (+ central) release setup:

     Execute the setuprel command in the local release directory. If a centrally
     installed release with the same version as the local one exists, it is set
     up, too. (If a release version is given as argument this is used as version
     for the central release instead of the one matching the local release.)

   - Central release setup (without a local release):

     Execute the setuprel command outside a local release directory with the
     central release version as argument.


For developers
++++++++++++++

.. describe:: addpkg

  This command adds the source code of the given package from the code
  repository to the local release directory. It has to be called in the local
  release directory with the name of one package.

.. describe:: checkstyle

  The checkstyle tool checks the layout of C++ and python code and reports
  changes that the fixstyle tool would apply.

  By default it checks all C++ and python files in the current directory and
  its subfolders recursively. Individual files can be checked explicitly by
  giving them as argument.

  .. note:: No commits can be pushed to the server if checkstyle or fixstyle
     report any problems

.. describe:: cleanrel

  This command deletes all built includes, object files, libraries, modules,
  and executables of your current local release.  The prompt for confirmation
  can be disabled with the -f option.

  -f
     Don't ask for confirmation

.. describe:: fixstyle

  ::

    Usage: fixtyle [-n|-p [-d command]] [files]

  The fixstyle tool formats the layout of C++ and python code.  It helps
  developers to achieve a common style of all Belle II software.

  By default it checks all C++ and python files in the current directory and
  its subfolders recursively.  Individual files can be checked explicitly by
  giving them as argument.

  -n
     If this option is given fixstyle opnly prints the changes which would be
     applied but the files are not modified. The return code indicates the
     number of files that would be changed.
  -p
     This option is equivalent to ``-n`` except that it will print the
     pep8 output instead of the code changes
  -d command
     This option can be used to specify the diff command that is called to
     report changes. Has to be given after the ``-n`` or ``-p``
     option.

  .. note:: No commits can be pushed to the server if checkstyle or fixstyle
     report any problems


.. describe:: listpkgs

  ::

      Usage: listpkgs [-l] [-s]

  This command lists the available packages.
  It has to be called in the local release directory.

  -l
    Also print the responsible librarians.
  -s
    Exclude locally installed packages


.. describe:: newmod

   ::

      Usage: newmod modulename

   This command create a source code template for a new C++ Module. It will ask
   some questions about the description, parameters, input and output elements
   as well as required functions and try to provide a full template header and
   implementation file where only the function bodies need to be implemented by
   the user.

.. describe:: newrel

   ::

      Usage: newrel directory [release]

   - This command creates a local directory with the given name
     as basis for a working copy of the Belle II software.
     It also prepares the build system.
   - If the basis for the code development should be a particular release,
     the version can be given as second argument.
   - If no second argument is given, the latest version of the code
     (head of git master) is taken.

.. describe:: prepare_belle2.sh

   ::

      Usage: prepare_belle2.sh [--non-interactive] [--optionals]

   If executed without arguments it will check if all necessary packages are
   installed and if not it will ask the user if it should do it.

   If --non-interactive is given it will not ask but just install the necessary
   packages but not the optional ones. If --optionals is given as well it will
   install everything without asking.

.. describe:: tagpkg

   ::

      Usage: tagpkg ["major"/"minor"/"patch"(=default)/tag]

   - This command tags the current version of the source code of a package
     and pushes the tag to the central repository.  It has to be called in the
     package directory of the local release.  There should be no locally
     modified files.
   - If no argument is given, the tag name is chosen automatically by
     increasing the patch level number, e.g. from ``v01-01-01`` to ``v01-01-02``.
   - If "minor" is given as argument, the minor version number is increased,
     e.g. from ``v01-01-01`` to ``v01-02-01``.
   - If "major" is given as argument, the major version number is increased,
     e.g. from ``v01-01-01`` to ``v02-01-01``.
   - Alternatively the name of the tag can be given explicitly as argument.

.. _Git/Stash Introduction: https://confluence.desy.de/x/2o4iAg

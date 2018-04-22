.. _local-install-precompiled:

Local installation with precompiled binaries
============================================

If you run on one of the supported distributions for which we provide
precompiled binaries you can install the software rather quickly with a few
commands. This only works if your machine is running one of the following
distributions. Otherwise please refer to :ref:`local-installation-source`

.. include:: supported-distributions.rst-fragment

After you installing the :ref:`belle2-tools` you need only to download the
software and corresponding externals version.

Downloading the Software Release
--------------------------------

We can obtain a list of available releases by just calling ::

  $ get_release.sh

This will give you a list of all release versions available for download. If
you don't know which version to use just take the one with the highest number
starting with ``release-``. You can then download this release by calling ::

  $ get_release.sh <version> <system>

where ``<version>`` should be the version you want and ``system`` should be the
short distribution name for your system from the list of supported
distributions in the table above.  For example, to download
``release-01-00-00`` on an Ubuntu 16.04 machine you need to run ::

  $ get_release.sh release-01-00-00 ubuntu1604

Downloading the Externals Software
----------------------------------

After downloading the software itself you will also need to download the
corresponding externals package which contains all external software required
by the Software. The easiest way to find out which externals version you need
is by trying to setup the software version you just downloaded. For example ::

  $ setuprel release-01-00-00

should print an error like

  The externals version v01-05-02 does not exist. You can use 'get_externals.sh' to install them.

which tells us that we need externals version ``v01-05-02`` by calling ::

  $ get_externals.sh v01-05-02 <system>

``system`` should be the short distribution name for your system from the list of
supported distributions in the table above. For example, to download
externals ``v01-05-02`` on an Ubuntu 16.04 machine you need to run ::

  $ get_externals.sh v01-05-02 ubuntu1604

Now everything should be installed and you can setup the software using ::

  $ setuprel release-01-00-00

(or any other version you installed).

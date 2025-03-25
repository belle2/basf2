.. _bat_installation:

Installation instructions
#########################

Platforms
---------

BAT has been developed to run on Linux and Max OSX. Windows is not
supported.

Dependencies
------------

1. BAT requires ROOT.

2. To install BAT from the development version, some additional packages
   are required, which can be installed (on Linux systems) via

   .. code-block:: sh

      sudo apt-get install autoconf automake git-core libtool

   Building and installing works with autoconf >= 2.63 and automake >=
   1.10. To run the tests, a more recent automake version is needed,
   v1.15 is known to be sufficient.

3. For model integration (and comparison) it is also useful to install
   Cuba, a library containing general-purpose multidimensional
   integration algorithms. It is recommended that you let BAT install
   Cuba for you.

Obtaining and Building BAT
--------------------------

Full instructions on obtaining and installing BAT are available
`here <https://bat.github.io/bat-docs/master/manual/html/cha-install.html>`_.

In short, to obtain BAT from github and prepare it for installation, run

.. code-block:: sh

   git clone http:s//github.com/bat/bat
   cd bat
   ./autogen.sh

Next you must configure it. The recommended configuration is

.. code-block:: sh

   ./configure --prefix=`pwd` --with-cuba=download

to install into the BAT directory itself. BAT will find your ROOT
installation and, with the above configure option, download and
install Cuba into the BAT directory for you.

To build and install, run

.. code-block:: sh

   make
   make install

Upon successful completion, there will be screen output with
environment-variable-setting commands set to have make BAT easily
accessible.

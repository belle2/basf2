Installation instructions
#########################

The package has an external dependency on ROOT and TCLAP.
Assuming these are installed in your system, you can compile all
examples by executing the following commands from the ``minuit``
subdirectory of the ``b2-fitting-training`` repository:

.. code-block:: sh

   mkdir build
   cd build
   cmake ../
   make
   cd -

If TCLAP is not installed in your system, download it from `<http://tclap.sourceforge.net>`_
(TCLAP is a header-only library and does not need to be compiled, downloading it is enough).
Then configure cmake with

.. code-block:: sh

   cmake -D TCLAP_PATH=path_to_tclap_dir ../

If you work on KEKCC, setup a recent ``basf2`` release to get ROOT, and
configure cmake by explicitly setting ``gcc``/``g++`` as compilers with

.. code-block:: sh

   cmake -D CMAKE_C_COMPILER=`which gcc` -D CMAKE_CXX_COMPILER=`which g++` -D TCLAP_PATH=path_to_tclap_dir ../

The compiled executables will be in the ``bin`` directory. Usage instruction
are printed when running the example with the ``-h`` argument, e.g.

.. code-block:: sh

   ./bin/simple-1d-fit -h

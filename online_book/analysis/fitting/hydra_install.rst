Installation instructions
#########################

The package has an external dependency on Hydra, ROOT and TCLAP.
To fully exploit Hydra's features you may need to install additional libraries.
For example, CUDA is required to use Nvidia GPUs. Please refer to the instructions
provided in the `Hydra GitHub repository <https://github.com/MultithreadCorner/Hydra>`_
for more information.

You can download Hydra from GitHub

.. code-block:: sh

	git clone https://github.com/MultithreadCorner/Hydra.git

The examples in this repository have been tested with Hydra release 3.2.1, but you are advised
to use the master branch. If you experience any problems using the master branch, please
open a GitLab issue so that we can keep the examples up-to-date with recent versions of Hydra.
To switch to release 3.2.1, instead, just do

	cd Hydra
	git checkout -b v3.2.1

If TCLAP is not installed in your system, download also TCLAP from `<http://tclap.sourceforge.net>`_ 
(Hydra and TCLAP are header-only libraries and do not need to be compiled, downloading them is enough).

Assuming you have ROOT installed, you can compile all fitting examples by executing the following
commands from the ``hydra`` subdirectory of the ``b2-fitting-training`` repository:

.. code-block:: sh

   mkdir build
   cd build
   cmake -D HYDRA_INCLUDE_DIR=path_to_hydra_dir -D TCLAP_INCLUDE_DIR=path_to_tclap_dir ../
   make
   cd -

If you work at KEKCC, setup a recent ``basf2`` release to get ROOT and
configure cmake by explicitly setting ``gcc``/``g++`` as compilers with

.. code-block:: sh

   cmake -D CMAKE_C_COMPILER=`which gcc` -D CMAKE_CXX_COMPILER=`which g++` -D HYDRA_INCLUDE_DIR=path_to_hydra_dir -D TCLAP_INCLUDE_DIR=path_to_tclap_dir ../

For each example program there is always a ``.cu`` and a ``.cpp`` source file in the ``src`` directory.
These file have the same include statement of a ``.inl`` file containing the actual source code implementing the program.
Cmake determines which backends are available and generates a Makefile to compile the code accordingly.
The compiled executables will be in the ``bin`` directory. For each example you will find different executables
compiled against the available backends. The name convention is ``example_{cpp,tbb,omp,cu}`` for standard (single-threaded) C++,
TBB (multi-threaded CPU), OpenMP (multi-threaded CPU), CUDA (Nvidia GPU) executables, respectively. Usage instruction
are printed when running each example with the ``-h`` argument, e.g.

.. code-block:: sh

   ./bin/simple-1d-fit_cpp -h

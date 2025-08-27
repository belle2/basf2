Before You Start
================

How to run the examples
-----------------------

The repository is organized in directories containing the examples corresponding to different fitting frameworks.

Most examples should work with a recent version of the ``basf2`` externals.
You may need to install the external fitting libraries depending on your tool of choice (but we will provide instructions, don't worry!).
Probably the easiest thing to do to obtain the latest externals, is to simply run:

.. code:: sh

   b2setup <a recent release>

We assume you run the script from each directory. For example, suppose
you were interested in RooFit, then:

.. code:: sh

   cd roofit
   python3 simple-1d-fit.py

If you want to do something else, you’ll have to check the paths to the input data.

For the zfit examples you may need to install zfit first, since at the time of writing it was not included in the externals:

.. code:: sh

   pip3 install --user zfit

The examples based on Minuit and Hydra rely on external libraries and need to be compiled. Please follow the instructions in the respective sections of the documentation.


How to build the documentation locally
--------------------------------------

From the root directory of this repository, simply run

.. code:: sh

   make html

The output is written to ``_build/html``. To open the starting page in your default browser, run

.. code:: sh

   ${BROWSER} _build/html/index.html

Stuck?
------

Please use `questions.belle2.org <https://questions.belle2.org>`__ to ask for help! Don’t be shy.

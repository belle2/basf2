.. _skim:

Skims
=====

.. warning::
        Skim package sphinx documentation is currently in progress. What is here is correct, but you should take a look at the `confluence page <https://confluence.desy.de/display/BI/Skimming+Homepage>`_ for more information about more skims than is listed here.


The skim package is a collection of high-level analysis scripts that reduce the data set to a manageable size by applying a simple selection.
The input to a skim are `mdst` files of processed data. The output is a so-called udst (micro-dst) which actually contains more information but fewer events.
That is: a udst file contains the same as an `mdst` but with the addition of `ParticleList`_ s of candidates build using the `modularAnalysis` tools.

.. _ParticleList: https://b2-master.belle2.org/software/development/classBelle2_1_1ParticleList.html

To get information about existing skim, ask the current skim group liaison.


Systematics skims
-----------------

.. automodule:: skim.systematics
    :members:


Physics skims
-------------

:math:`B\to\textrm{charm}` skims
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: skim.btocharm
    :members:

:math:`B\to\textrm{charmless}` skims
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: skim.btocharmless
    :members:

Charm physics skims
~~~~~~~~~~~~~~~~~~~

.. automodule:: skim.charm
    :members:

.. Commented out, because this module no longer exists
   Charmonium skims
   ~~~~~~~~~~~~~~~~

   .. automodule:: skim.charmonium
       :members:

Dark sector skims
~~~~~~~~~~~~~~~~~

.. automodule:: skim.dark
    :members:

Full event interpretation skims
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: skim.fei
    :members:
    :undoc-members:

Leptonic skims
~~~~~~~~~~~~~~

.. automodule:: skim.leptonic
    :members:
    :undoc-members:
    
Quarkonium skims
~~~~~~~~~~~~~~~~

.. automodule:: skim.quarkonium
    :members:

Radiative and electroweak penguins skims
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: skim.ewp
    :members:

Semileptonic skims
~~~~~~~~~~~~~~~~~~

.. automodule:: skim.semileptonic
    :members:
    :undoc-members:

Time-dependent CP-violation (TDCPV) analysis skims
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: skim.tcpv
    :members:

Tau-pair skims
~~~~~~~~~~~~~~~~~

.. automodule:: skim.taupair
    :members:


For skim liaisons and developers
--------------------------------

Some functions and tools are for the use of skim liaisons and developers.

.. tip:: If you are only interested in the selection criteria this section is probably not relevant for you.


.. automodule:: skim.registry
    :members:
 

.. automodule:: skimExpertFunctions
    :members:
    :undoc-members:

Skim performance statistics
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following two command-line tools can be used to test the performance of a skim. They are
available in `skim/tools/stats`.

They both `skim.registry`


``runSkimsForStats.py``: Run skim scripts and save output
.........................................................

.. argparse::
   :filename: skim/tools/stats/runSkimsForStats.py
   :func: getArgumentParser
   :prog: runSkimsForStats.py
   :nodefault:
   :nogroupsections:


``printStatsSkims.py``: Print tables of performance statistics
..............................................................

.. argparse::
   :filename: skim/tools/stats/printSkimStats.py
   :func: getArgumentParser
   :prog: printSkimStats.py
   :nodefault:
   :nogroupsections:


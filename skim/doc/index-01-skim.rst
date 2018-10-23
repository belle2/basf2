.. _skim:

Skims
=====

.. warning::
        Skim package sphinx documentation is currently in progress. What is here is correct, but you should take a look at the `confluence page <https://confluence.desy.de/display/BI/Skimming+Homepage>`_ for more information about more skims than is listed here.


The skim package is a collection of high-level analysis scripts that reduce the data set to a manageable size by applying a simple selection.
The input to a skim are `mdst` files of processed data. The output is a so-called udst (micro-dst) which actually contains more information but fewer events.
That is: a udst file contains the same as an `mdst` but with the addition of `ParticleList`_ s of candidates build using the `modularAnalysis` tools.

.. _ParticleList: https://b2-master.belle2.org/software/development/classBelle2_1_1ParticleList.html

To get information about existing skim, ask current skim group convener.


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


Leptonic skims
~~~~~~~~~~~~~~

.. automodule:: skim.leptonic
    :members:

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

Time-dependent CP-violation (TDCPV) analysis skims
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: skim.tdcpv
    :members:

Tau-pair skims
~~~~~~~~~~~~~~~~~

.. automodule:: skim.taupair
    :members:

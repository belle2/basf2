KLM (:math:`K_{L}^0` and Muon Detector)
=======================================

In the following we describe the ``klm`` package (documentation still under development).


Modules
-------

This is a list of the ``klm`` modules. Some modules (in the ``klm/bklm`` and ``klm/eklm`` subfolders) that are currently used only for debugging or specific studies are omitted here.

.. b2-modules::
   :package: klm
   :regex-filter: ^KLM
   :io-plots:

.. b2-modules::
   :modules: MCMatcherKLMClusters 
   :io-plots:


Clusterization
--------------

Few words about how a ``KLMCluster`` is built starting from ``BKLMHit2d`` and ``EKLMHit2d`` are more than desiderable.


Muon identification
-------------------

It would be veeery nice to document here the algorithm for the muon identification (and the ``MuidBuilder`` class).


Calibration
-----------

This seems the best place to cite our internal note `KLM offline calibration`_.

.. _KLM offline calibration: https://docs.belle2.org/record/1848?ln=en


Tools
-----

Section for our beautiful tools.


``b2klm-create-dqm``: Create DQM reference plots
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. argparse::
   :filename: klm/tools/b2klm-create-dqm
   :func: arg_parser
   :prog: b2klm-create-dqm
   :nodefault:
   :nogroupsections:


``b2klm-mask-dqm``: Mask very hot channels in DQM reference plots
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. argparse::
   :filename: klm/tools/b2klm-mask-dqm
   :func: arg_parser
   :prog: b2klm-mask-dqm
   :nodefault:
   :nogroupsections:

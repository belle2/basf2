
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

Few words about how a ``KLMCluster`` is built starting from ``KLMHit2d`` are more than desirable.


Muon identification
-------------------
.. toctree:: MuonIdentification


Calibration
-----------
.. toctree:: Calibration


Tools
-----

``b2klm-create-dqm``: Create DQM reference plots
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. argparse::
   :filename: klm/tools/b2klm-create-dqm
   :func: arg_parser
   :prog: b2klm-create-dqm
   :nodefault:


``b2klm-mask-dqm``: Mask very hot channels in DQM reference plots
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. argparse::
   :filename: klm/tools/b2klm-mask-dqm
   :func: arg_parser
   :prog: b2klm-mask-dqm
   :nodefault:


Disclaimers for development
----------------------------

Charge or Pulse Height from Feature Extraction: 
- Due to (scintillator) firmware anamolies, there are unphysical peaks near 0 and ``> 400`` ADC counts. 
   - Recommended action is to suppress these peaks or replace them with NaNs. 
- The peaks at charge ``> 400`` ADC counts have been a result overlapping ramps negatively shifting the output voltages (lower Wilkinson ADC counts). 
   - The KLM firmware was updated on Exp 40 Run 858 to properly address this bug. 
   - Details are outlined `here <https://indico.belle2.org/event/17965/contributions/109456/attachments/40138/59714/cketter_klm_weekly_260330.pdf>`.
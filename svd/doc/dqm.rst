.. _svddqm:

SVD DQM
=======

The SVD DQM modules produce plots to be assess the quality of the data. 
Only ``SVDunpackerDQM`` is executed on HLT, the rest of the modules are executed on ExpressReco on a fraction of events.

DQM Modules
-----------

This is a list of the ``svd`` dqm modules. 

.. b2-modules::
   :package: svd
   :modules: SVDDQMClustersOnTrack, SVDDQMEfficiency, SVDDQMHitTime, SVDDQMExpressReco, SVDDQMInjection, SVDUnpackerDQM
   :io-plots:

DQM Analysis Modules
--------------------

.. b2-modules::
   :package: dqm
   :modules: DQMHistAnalysisSVDEfficiency, DQMHistAnalysisSVDGeneral, DQMHistAnalysisSVDOnMiraBelle
   :io-plots:

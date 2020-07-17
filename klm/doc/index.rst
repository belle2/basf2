
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
Muon identification in KLM uses differences in longitudinal penetration depth and transverse scattering of the extrapolated track.
The muid reconstruction module is the tracking package of ``BASF2`` and procedes in two steps:

1. **Track extrapolation** using the muon hypothesis only;
2. **Likelihood extraction** for each of six particle hypothesis: :math:`\mu`, :math:`\pi`, :math:`K`, :math:`p`, :math:`d`, :math:`e`.

The six likelihoods that are assigned to a given track are stored as log-likelihood values in the ``KLMMuidLikelihood`` data-object. In the post-reconstraction analysis,
the log-likelihood differences may be used to selector reject the muon hypotesis for a give track.

.. b2-modules::
   :modules: Muid
   :io-plots:
   :no-parameters:
   
Extrapolation
~~~~~~~~~~~~~
The **extrapolation** proceeds step by step through the detector geometry, starting at the outermost point of the reconstructed track's trajectory and with phase-space coordinates and covariance matrix.
Upon crossing a KLM detector layer, the nearest two-dimensional hit -if any- in that layer is considered for association with the track. If the hit is within about :math:`3.5\sigma`
(where :math:`\sigma` is the 2d hit uncertainty) in either of the two local-coordinates directions then it is declared a matching hit and the Kalman filter 
uses it to adjust the track properties before the next step in extrapolation.
At the same time, the Kalman filter's fit quality (:math:`\chi^{2}`) is accumulated for the track.

The extrapolation ends when the kinetic energy falls below a user-defined threshold (nominally 2 MeV) or the track curls  inward to a cylindrical radius below 
the beam pipe one or the tracks escapes from KLM.
If the track reached the KLM, it is classified according to how and where the extrapolation ended. 

Likelihood extraction
~~~~~~~~~~~~~~~~~~~~~
The likelihood of having the matched-hit range and transverse-scattering :math:`\chi^{2}` distribution is obtained from pre-calculated probability density functions (PDFs). 
There are separate PDFs for each charged-particle hypothesis and charge and for each extrapolation outcome. 

* The *longitudinal-profile* PDF value for extrapolation ending outcome and outermost layer and for particle hypotesis is samples according to the measurement vector :math:`\vec{c}` given by: (a) the pattern of of all KLM layers touched during the extrapolation and (b) the pattern of matched hits in the touched layers.

* The *transverse-scattering* probability density function for KLM region and particle hypotesis is sampled according to the measurement of :math:`\chi^{2}` from the Kalman filter and the number of degrees of freedom, which is twice the number of matched hits.

For each track, the **likelihood** for a given particle hypothesis is the product of the corrisponding longitudinal-profile and transverse-scattering PDF values.
The natural logarithm of this value is stored in the ``KLMMuidLikelihood``  data-object. Then, the six likelihood values are normalized by dividing by their sum 
and stored in the ``KLMMuidLikelihood`` data-object.

The log-likelihood difference :math:`\Delta` is the most powerful discriminator between the competing hypotesis. 
The requirement :math:`\Delta > \Delta_{min}` for a user-selected :math:`\Delta_{min}` provides the best signal efficiency for the selected backgrund rejection.

Muid Likelihoods are constructed by ``MuidBuilder`` class.

.. cpp:class:: MuidBuilder 
Build the Muid likelihoods starting from the hit pattern and the transverse scattering in KLM

REGISTER_FUNCTION("fillPDFs(Hypotesys)", fillPDFs, R"DOC(
Retrieve the PDFs from the database according to the given hypothesis.

Parameters:
   Hypothesis (int): hypothesis number.

.. see also:: `"MuidElementNumber Class" <https://b2-master.belle2.org/software/development/classBelle2_1_1MuidElementNumbers.html#abac5ea6b84578687bc483bb611738e35>`_


 )DOC")


.. seealso:: `"Track extrapolation and muon identification using GEANT4E in event reconstruction in the Belle II experiment" <https://docs.belle2.org/record/502/files/BELLE2-TALK-CONF-2017-026.pdf>`_

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

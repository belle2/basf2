.. _analysis_continuumsuppression:

Continuum suppression
=====================

This page contains instructions on how to use the continuum suppression
framework, with a focus on recent modifications. For a detailed description of
the variables, please refer to Chapter 9 (Background suppression for B decays)
of `The Physics of the B Factories book <https://arxiv.org/abs/1406.6311>`_

Example usage
-------------

In order to build continuum suppression variables, you need to first
reconstruct a B on the signal side, then reconstruct the rest of event (ROE) `buildRestOfEvent`,
and eventually build the continuum suppression variables `buildContinuumSuppression`.

Since the ROE can be affected by background and noise, a mask has to be
provided to try to get rid of part of this background `appendROEMasks`.
The mask defines a set
of cuts which will be applied on ROE objects, and only the ROE objects passing
the selection mask will be used to build the continuum suppression variables.

The generic interface is the following:

::

  # build your signal ('B0')

  buildRestOfEvent('B0', path=main)
   
  cleanMask = ('cleanMask', '<Your selections for ROE>')
  appendROEMasks('B0', [cleanMask], path=main)
   
  buildContinuumSuppression('B0', 'cleanMask', path=main)

Where ``<Your selections for ROE>`` is a set of cuts on tracks and clusters of ROE
which are in general analysis dependent. Some examples will be given below.

.. autofunction:: modularAnalysis.buildContinuumSuppression
   :noindex:

The ROE mask cuts should be tuned for each individual physics analysis.
However, it may be a good idea to always require a minimum of 1 CDC hit for the charged ROE particles to exclude VXD-only fake tracks.
Here is a simple example that you can use as a starting point:

::

  cleanMask = ('cleanMask', 'nCDCHits > 0 and useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')

The default `CleoConeCS` variable returns the cones calculated from all final state
particles. It is now possible to construct CLEO Cones using only particles in
the ROE. If you want to store the CleoCones constructed using only the ROE
particles, you simply need to add ``ROE`` as a second argument to your variable:

::

  variables = ['CleoCone(1)','CleoCone(1,ROE)']

Note that you can store both types of CleoClones in a single ntuple.

There is also the option to calculate the KSFW moments (:b2:var:`KSFWVariables`)
constructed from the reconstructed B-mesons final state particles.
In Belle, this was possible, but it was not often employed as the KSFW moments
become analysis dependent which is not good for systematics.
For this reason, the call to the :b2:var:`KSFWVariables`
returns the variables calculated from the B-meson primary daughters. If you
would like to store the :b2:var:`KSFWVariables` constructed from the B final state
particles, you need to add ``FS1`` as an additional argument (``FS1 = final_state_1``,
from the Belle software):

::

  variables = ['KSFWVariables(hso00)','KSFWVariables(hso00,FS1)']

Again, as shown in this example, you can store both cases in your ntuple.

Continuum Suppression variables
-------------------------------

The Continuum Suppression variables are defined as following:

Thrust and thrust axis
  For a set of :math:`N` particles with momenta :math:`p_i` the thrust axis :math:`\vec{T}` is defined as the unit vector along which their total projection is maximal.
  The thrust scalar is 
  :math:`T=\frac{\sum^N_{i=1} |\vec{T}\cdot \vec{p}_i|}{\sum^N_{i=1} |\vec{p}_i|}`,

CLEO Cones 
  The CLEO collaboration introduced variables based on the sum of the
  absolute values of the momenta of all particles within angular sectors around the thrust
  axis in intervals of 10 degrees, resulting in 9 concentric cones.

Fox-Wolfram moments
  For a set of :math:`N` particles with momenta :math:`p_i`, the l-th order Fox-Wolfram moment is defined as
  :math:`H_l = \sum^N_{i,j=1}|\vec{p}_i||\vec{p}_j| P_l (\cos{\theta_{i,j}})`,
  where :math:`P_l` are Legendre polynomials and :math:`\theta_{i,j}` is the angle between the particles


Deep Continuum Suppression
--------------------------

The Deep Continuum Suppression (DCS) employs additional detector-level
variables describing nearly every track (cluster) in the event to increase the
classification performance.

It is described in detail in this `MsC thesis <http://ekp-invenio.physik.uni-karlsruhe.de/record/48934>`_.
Tutorial files are available in
``basf2`` in ``analysis/examples/tutorials/``.

There are two big differences when using the DCS instead of the Continuum Suppression:

  1 Writing out of new variables, which describe single tracks and clusters instead of the whole shape of the event.

  2 Using Deep Neural Networks as MVA methods to increase performance and to deal with the large number of new variables.

This following section provides additional information about the DCS, which supplements the information in the tutorials.

Adversarial Networks
""""""""""""""""""""

Due to the new variables in the DCS, correlations between the classifier output
and quantities like ``Mbc`` and :math:`\Delta{Z}` are much more likely to occur.

Using Adversarial Networks during training can reduce such correlations to a
minimum. This is achieved by using additional networks for signal and
background distributions of each quantity to train against the regular Neural
network used for classification.

In the DCS, the impact on these additional Adversarial Networks can be
regularized with the parameter :math:`\lambda`.

This parameter is highly dependent on the given problem and can vary in orders of magnitude.

Please note that in most cases either the signal or continuum distribution of a quantity is correlated.

While in the DCS tutorial there is an Adverserial Network for every signal and
background distribution for every quantity (which is put in as a spectator),
one should limit the number of Adversarial Networks to only those distributions
which are correlated.

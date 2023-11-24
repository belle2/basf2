.. _tracking_ckf:

Inter-Detector Track Finding
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. warning::
  This documentation is under construction!

The inter-detector track finding is done through a `Combinatorial Kalman Filter (CKF) <http://linkinghub.elsevier.com/retrieve/pii/0168900287908874>`_. We have several implementations of the CKF, depending on the detector we are extrapolating to.

The base of our CKF is a generic *state* object. A state in a CKF can either be based on the :ref:`RecoTrack<recotrack>` that is extrapolated in the current detector, or the hits in the detector that is extrapolated to. These states are from here on referred to as track-based and hit-based, respectively.

The base concept of the CKF in basf2 is described here briefly. The first step is the *relation creation*:

1. Use the track-based states to build relations to hit-based states, applying rather loose cuts to build and keep all the correct relations.

2. Build relations between hit-based states, again applying rather loose cuts.

This marks the beginning of the combinatorial part in CKF. In the following steps the relations are followed to build the tracks. However, the implementation of the two steps can vary. For both the :ref:`SVD to PXD CKF<tracking_svd2pxd_ckf>` and the :ref:`CDC to SVD CKF<tracking_cdc2svd_ckf>` the relations are created all at once at the beginning, while for the :ref:`SVD to CDC CKF<tracking_svd2cdc_ckf>` and the :ref:`ECL to CDC CKF<tracking_ecl2cdc_ckf>` the relations are only build step by step ("on-the-fly") to reduce the number of possible combinations significantly, as far less relations between individual hit-bases states are created. Note that during the relation creation it is possible to "jump" over a layer if the setup allows for this. This might be necessary to account for e.g. insensitive or noisy detector parts, e.g. where sensors are stitched together, broken electronics, etc.

The relation creation is followed by a *tree search*. Beginning with the track-based *seed state** all related hit-bases states are probed. The *seed state* marks the beginning of a *path* that is followed, and over time more and more states are added to the path. In the terminology of the CKF, the current state is the *parent state*, or just referred to as *current state*, while all the states to be probed, i.e. the candidates, are referred to as *child states*. Each child state is probed individually using a set of filters, and if it is found to be worthy child state by passing all filters and being among the ``n`` best candidates, it is added to the pat* which is then used as the next parent state and all the hit-states related to the last state in the path are the next child states to be probed. This process continues until no more states can be added to a given path, either because we leave the current detector (= we are at the innermost or outermost layer of that detector, depending on the direction), or because no more child states are available e.g. because the current state is a dead end (bad path) or because the current detector has insensitive or inefficient regions in the direction of extrapolation. As the ``n`` best candidates can be added to a path in each step, a tree is build for each track-based seed state. Hence the name *tree search*. In the end, only the best final paths are considered for a final check.

As the relation creation before, the tree search differs for the CKFs. In the two cases where all relations are built at the beginning, all relations are available at the start of the tree search so the trees are just traversed based on the relations. In the two cases where the relations are build on-the-fly, the relation creation is embedded in the tree search: at the end of the current path, possible child states are defined and probed immediately.

There usually are five filters after relation creation to decide which child states to keep and which to discard. These filters are the KF part of CKF:

1. *First Filter*: The first filter uses information of the current path to discard child states that.

2. *Advance Filter*: An extrapolation filter extrapolates the path to the layer of the probed child hit. The child hit can be discarded e.g. if the extrapolation fails.

3. *Second Filter*: The second filter now uses information from the extrapolation to probe the child states. For example, this can be the distance between the child state hit position on the detector surface (for SVD and PXD) and the extrapolated position.

4. *Update Filter*: If a child state passed all the previous filters, it is now included in the path temporarily and a full fit with the child state is performed.

5. *Third Filter*: The third filter can now use information from the fit like the fit's :math:`\chi^2`.

Each of the filters thus has different information available to decide whether or not to discard a child hit.

After the tree has been traversed and no more hit-bases states can be added to any path, a final *result filter* is employed. It checks for all the paths belonging to a given seed state which is the best path overall. Only the best overall path for each seed is kept.

These are the CKF algorithms that are used in basf2. Each section contains more information about the specific implementation.

.. _tracking_svd2pxd_ckf:

SVD to PXD CKF
""""""""""""""

.. _tracking_svd2cdc_ckf:

SVD to CDC CKF
""""""""""""""

.. _tracking_cdc2svd_ckf:

CDC to SVD CKF
""""""""""""""

.. _tracking_svdcdc_merger_ckf:

SVD and CDC merger CKF
""""""""""""""""""""""

.. _tracking_ecl2cdc_ckf:

ECL to CDC CKF
""""""""""""""

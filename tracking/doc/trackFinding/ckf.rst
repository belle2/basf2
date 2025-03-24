.. _tracking_ckf:

Inter-Detector Hit Finding
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. warning::
  This documentation is under construction!

The inter-detector track finding is done through a `Combinatorial Kalman Filter (CKF) <http://linkinghub.elsevier.com/retrieve/pii/0168900287908874>`_. We have several implementations of the CKF, depending on the detector we are extrapolating to.

The base of our CKF is a generic *state* object. A state in a CKF can either be based on the :ref:`RecoTrack<recotrack>` that is extrapolated in the current detector, or the hits in the detector that is extrapolated to. These states are from here on referred to as track-based and hit-based, respectively.

The base concept of the CKF in `basf2` is described here briefly. The first step is the *relation creation*:

1. Use the track-based states to build relations to hit-based states, applying rather loose cuts to build and keep all the correct relations.

2. Build relations between hit-based states, again applying rather loose cuts.

This marks the beginning of the combinatorial part in CKF. In the following steps the relations are followed to build the tracks. However, the implementation of the two steps can vary. For both the :ref:`SVD to PXD CKF<tracking_svd2pxd_ckf>` and the :ref:`CDC to SVD CKF<tracking_cdc2svd_ckf>` the relations are created all at once at the beginning, while for the :ref:`SVD to CDC CKF<tracking_svd2cdc_ckf>` and the :ref:`ECL to CDC CKF<tracking_ecl2cdc_ckf>` the relations are only build step by step ("on-the-fly") to reduce the number of possible combinations significantly, as far less relations between individual hit-bases states are created. Note that during the relation creation it is possible to "jump" over a layer if the setup allows for this. This might be necessary to account for e.g. insensitive or noisy detector parts, e.g. where sensors are stitched together, broken electronics, etc.

The relation creation is followed by a *tree search*. Beginning with the track-based *seed state* all related hit-bases states are probed. The *seed state* marks the beginning of a *path* that is followed, and over time more and more states are added to the path. In the terminology of the CKF, the current state is the *parent state*, or just referred to as *current state*, while all the states to be probed, i.e. the candidates, are referred to as *child states*. Each child state is probed individually using a set of filters, and if it is found to be worthy child state by passing all filters and being among the ``n`` best candidates, it is added to the path* which is then used as the next parent state and all the hit-states related to the last state in the path are the next child states to be probed. This process continues until no more states can be added to a given path, either because we leave the current detector (= we are at the innermost or outermost layer of that detector, depending on the direction), or because no more child states are available e.g. because the current state is a dead end (bad path) or because the current detector has insensitive or inefficient regions in the direction of extrapolation. As the ``n`` best candidates can be added to a path in each step, a tree is build for each track-based seed state. Hence the name *tree search*. In the end, only the best final paths are considered for a final check.

As the relation creation before, the tree search differs for the CKFs. In the two cases where all relations are built at the beginning, all relations are available at the start of the tree search so the trees are just traversed based on the relations. In the two cases where the relations are build on-the-fly, the relation creation is embedded in the tree search: at the end of the current path, possible child states are defined and probed immediately.

There usually are five filters after relation creation to decide which child states to keep and which to discard. These filters are the KF part of CKF:

1. *First Filter*: The first filter uses information of the current path to discard child states that.

2. *Advance Filter*: An extrapolation filter extrapolates the path to the layer of the probed child hit. The child hit can be discarded e.g. if the extrapolation fails.

3. *Second Filter*: The second filter now uses information from the extrapolation to probe the child states. For example, this can be the distance between the child state hit position on the detector surface (for SVD and PXD) and the extrapolated position.

4. *Update Filter*: If a child state passed all the previous filters, it is now included in the path temporarily and a full fit with the child state is performed.

5. *Third Filter*: The third filter can now use information from the fit like the fit's :math:`\chi^2`.

Each of the filters thus has different information available to decide whether or not to discard a child hit.

After the tree has been traversed and no more hit-bases states can be added to any path, a final *result filter* is employed. It checks for all the paths belonging to a given seed state which is the best path overall. Only the best overall path for each seed is kept.

These are the CKF algorithms that are used in `basf2`. Each section contains more information about the specific implementation.

.. _tracking_cdc2svd_ckf:

CDC to SVD CKF
""""""""""""""

The *CDCToSVDSpacePointCKF* is the first CKF in the track finding chain. It takes all tracks found in :ref:`tracking_trackFindingCDC`, converts them into seeds, and extrapolates them into the SVD volume to add SVD :ref:`SVDSpacePoints<svdsps>` to the tracks. Thus, in this case the SVDSpacePoints are the hit states. The CDCToSVDSpacePointCKF uses the method of creating all the relations in advance and then follows the relations in the tree search. During relation creation, simple cuts on geometrical properties are employed. The *first*, *second*, and *third* filter applied during the tree search are all based on MVAs (using FastBDT), but also here simple cut based filters exist. Finally, the *result filter* also uses an MVA to decide which paths to keep and thus to convert to :ref:`RecoTracks<recotrack>`.

.. _tracking_svdcdc_merger_ckf:

SVD and CDC merger CKF
""""""""""""""""""""""

This CKF, called *CDCToSVDSeedCKF*, is used after the :ref:`CDCToSVDSpacePointCKF<tracking_cdc2svd_ckf>` and after the :ref:`standalone SVD track finding using the VXDTF2<tracking_trackFindingSVD>` were applied to all the :ref:`SVDSpacePoints<svdsps>`. Its goal is to combine the remaining :ref:`CDCRecoTracks<recotrack>` from the :ref:`tracking_trackFindingCDC` that do not have any SVDSpacePoints attached to them after the CDCToSVDSpacePointCKF with :ref:`SVDRecoTracks<recotrack>` from the SVD standalone tracking.

To do so, it first extrapolates both the CDC standalone tracks and the SVD standalone tracks onto the CDC inner wall. Afterwards it creates relations as well and performs the tree search. Since it is operating on existing SVD RecoTracks, the tree search is a lot simpler. The first, second, and third filter do not attempt to remove any hits but accept all of them. The final decision of the result filter on which combinations of CDCRecoTracks and SVDRecoTracks to combine is based on an MVA again.

.. _tracking_svd2cdc_ckf:

SVD to CDC CKF
""""""""""""""

Using the *ToCDCCKF*, all SVDRecoTracks from SVD standalone track finding that were not combined with an existing CDCRecoTrack before are now extrapolated into the CDC volume to attach CDC hits to them. Often these tracks have a rather low transverse momentum :math:`p_{T}` so that track parts in the CDC are often quite small. This makes it difficult for the CDC track finding to identify tracks. The goal is to improve the momentum resolution with the additional CDC hits.

To reduce the problem of combinatorics, this CKF does not create all the relations in advance before traversing the tree, but builds the relations and thus the tree in each step considering only the next possible hits. It loops over all the seeds and uses them as a starting point. Then, it creates all the possible next states from the CDC hits applying a loose preselection on the difference in :math:`\varphi` and the difference in CDC layer number, which are then filtered more thoroughly in the next steps. All selected states are then extrapolated and updated without a filter in between, so basically the *second filter* from above is skipped. Afterwards, a selection on the best fit results is performed and the next sub-tree is created and checked. Once all seeds were processed and extrapolated to the furthest possible extend, a final track selection is performed and the results are stored as RecoTracks.

.. _tracking_ecl2cdc_ckf:

ECL to CDC CKF
""""""""""""""

This CKF is still experimental and not actively used. The idea is that we might miss tracks that are very forward or backward and have rather low transverse momentum , e.g. electrons, during track reconstruction. These particles can nonetheless create ECLClusters that are reconstructed. The ECL clusters are used to estimate the momentum of the particle, based on the cluster energy and the location. The clusters are then used to create seed states, which are then extrapolated to the CDC as in the :ref:`ToCDCCKF<tracking_svd2cdc_ckf>`. More active development of this feature would be needed to improve the performance of this CKF before we would actively use it.

.. _tracking_svd2pxd_ckf:

To PXD CKF
""""""""""

The *ToPXDCKF* is the last step of the track finding chain and currently the only algorithm to add PXD hits to tracks. While the VXDTF2 used in the :ref:`tracking_trackFindingSVD` can in principle be used with PXD hits, this feature isn't used currently. Besides that, it is essentially working the same way as the :ref:`CDCToSVDSpacePointCKF<tracking_cdc2svd_ckf>` in the way the filters work and the type of the filters, i.e. the first, second, and third filter as well as the result filter are all using MVAs.

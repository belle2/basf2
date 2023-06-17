.. _trk_matching:

Track Matching
--------------

This section describes the definition of the various status, that the matching of tracks can produce.
The four main figures of merit for the track finder - the finding efficiency, the charge efficiency, hit efficiency, the clone rate and the fake rate - are defined using these matching labels as described below.


Overview: Available Status
^^^^^^^^^^^^^^^^^^^^^^^^^^

After running the TrackFinderMCTruthRecoTracksModule which creates Genfit Track Candidates from MC information (in the following called MC track candidates)
and the "normal" track finder algorithm which uses hit information from the detector (in the following called PR track candidates),
you can apply the MCRecoTracksMatcherModule, which creates relations between the two StoreArrays
of track candidates by looking at the hit content.

.. note::
  the charge of the track can be changed in two places after the pattern recognition step:

  1. the track fit can flip the track (in this case the seed charge and the track charge will differ)

  2. the flip&refit step can change the PR track charge, and the track will be then refitted


If the hit content of two track candidates has a non-zero intersection,
a relation is created with the ratio between the intersection number of hits and the total number of
hits in the candidate as a weight (in both directions because the weight can be different as the total number
of hits in a track can be different for MC and PR track candidates).
The weights from PR to MC track candidates are called purity, and from MC to PR track candidates efficiency.
Only the single highest value for each PR and MC track candidates is stored in the relation array
(so only the "best match" is stored) and only if the purity is above 2/3 and the efficiency is above 0.05.

After the matching, each PR and each MC track candidate is given a single label:

Tracks from Pattern Recognition (PRTracks) can be:

* matched,
* wrongCharge,
* clone,
* cloneWrongCharge
* background
* ghost

as it can be seen in the ``PRToMCMatchInfo`` of the helper class ``TrackMatchLookUp.h``.

Charged MCParticles (MCTracks) can be

* found or matched (we will call it found to not confuse with the PR track candidates),
* wrongCharge,
* merged,
* mergedWrongCharge
* missing.

as it can be seen in the ``MCToPRMatchInfo`` of the helper  ``TrackMatchLookUp.h``.

.. _trk_matching_analysis:

Reconstructed Track MC-Matching
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

We will first describe the labels here briefly
(as it can also be found in the comments in the ``MCMatcherTracksModule.h``)
and then show some examples.

The PR track candidate can be classified into six categories:

* MATCHED

  * The highest efficiency PR track candidate of the highest purity MC track candidate to this PR track candidate is the same as this PR track candidate,
    and the charge is correctly reconstructed.
    This means the PR track candidate contains a high contribution of only one MC track candidate and is also the best of all PR track candidates
    describing this MC track candidate.

* WRONG CHARGE

  * The highest efficiency PR track candidate of the highest purity MC track candidate to this PR track candidate is the same as this PR track candidate,
    BUT the charge is not correctly reconstructed.
    This means the PR track candidate contains a high contribution of only one MC track candidate and is also the best of all PR track candidates
    describing this MC track candidate, but the direction of the reconstructed track is wrong.

* CLONE

  * The highest purity MC track candidate has a different highest efficiency PR track candidate than this track, anyway the charge
    is correctly reconstructed.
    This means the PR track candidate contains high contributions of only one MC track candidate (with the correct charge) but a different other
    PR track candidate contains an even higher contribution to this MC track candidate.

* CLONE WRONG CHARGE

  * The highest purity MC track candidate has a different highest efficiency PR track candidate than this track, moreover the charge
    is NOT correctly reconstructed.
    This means the PR track candidate contains high contributions of only one MC track candidate (with the wrong charge) but a different other
    PR track candidate contains an even higher contribution to this MC track candidate.

* BACKGROUND

  * The PR track candidate contains mostly hits, which are not part of any MC track candidate.
    This normally means that this PR track candidates is made of beam background hits or random combinations of hits.
    Be careful: If e.g. only creating MC track candidates out of primary particles,
    all found secondary particles will be called background (which is happening in the default validation)

* GHOST

  * The highest purity MC track candidate to this PR track candidate has a purity lower than the minimal purity given in
    the parameter minimalPurity (2/3) or has an efficiency lower than the efficiency given in the parameter
    minimalEfficiency (0.05).
    This means that the PRTrack does not contain a significant number of a specific MCTrack nor can it considered only made of background.



MC Track Matching with Reconstructed Tracks
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

MC track candidates are classified into three categories:

* MATCHED/FOUND

  * The highest purity MC track candidate of the highest efficiency PR track candidate of this MC track candidate is the same as this MC track candidate,
    and the MC track charge and the PR track charge are the same.
    This means the MC track candidate is well described by a PR track candidate (including the charge) and this
    PR track candidate has only a significant contribution from this MC track candidate.

* WRONG CHARGE

  * The highest purity MC track candidate of the highest efficiency PR track candidate of this MC track candidate is the same as this MC track candidate,
    BUT the MC track charge and the PR track charge are NOT the same.
    This means the MC track candidate is well described by a PR track candidate (with the wrong charge) and this
    PR track candidate has only a significant contribution from this MC track candidate.

* MERGED

  * The highest purity MC track candidate of the highest efficiency PR track candidate of this MC track candidate is not the same as this MC track
    candidate, however the reconstructed charge is correct.
    This means this MC track candidate is mostly contained in a PR track candidate (including the charge),
    which in turn however better describes a MC track candidate different from this.

* MERGED WRONG CHARGE

  * The highest purity MC track candidate of the highest efficiency PR track candidate of this MC track candidate is not the same as this MC track
    candidate,  moreover the reconstructed charge is NOT correct.
    This means this MC track candidate is mostly contained in a PR track candidate (but with the wrong charge),
    which in turn however better describes a MC track candidate different from this.

* MISSING

  * There is no highest efficiency PR track candidate to this MC track candidate, which also fulfills the minimal purity requirement.


Four examples are shown in the pictures (charge check not included).
The circles on the left side shows the MC track candidates, the right side stands for the PR track candidates.
The arrows depict that there are common hits, the percentage values shows the ratio.

 
.. figure:: /tracking/doc/figures/fom_found.png
  :width: 40em
  :align: center

  There is a one to one connection between a MCTrackCand and a track from the track finder.
  The MCTrackCand is labeled found and the other track is labeled matched if the reconstructed
  charge is also correct. If the reconstructed charge is wrong, then the MCTrackCand and the PRTrack
  are labeled as wrongCharge.

.. figure:: /tracking/doc/figures/fom_clone.png
  :width: 40em
  :align: center

  The MCTrackCand is found twice. The track from the track finder with the higher percentage
  (the green one in this example) is labeled matched, the other one cloned (if the charge is correctly reconstructed).
  The MCTrackCand is nevertheless labeled found.

.. figure:: /tracking/doc/figures/fom_fake.png
  :width: 40em
  :align: center

  The track from the track finder is created with hits from many different MCTrackCands.
  As none of the corresponding hit ratios exceeds 66%, the track is called ghost.
  The hit ratios of the MCTrackCands itself do not play any role here.

.. figure:: /tracking/doc/figures/fom_background.png
  :width: 40em
  :align: center

  The found track does not describe any of the MCTrackCands well (or well enough) - but is made out of background hits.
  This track is also called background.


Figures of Merit
^^^^^^^^^^^^^^^^

The main figures of merit, as also shown on the validation side, are:

1) **Finding efficiency**: Number of MC track candidates which are labeled found or wrongCharge divided by the total number of MC track candidates
2) **Charge efficiency**: Number of MC track candidates which are labeled found divided by the total number of found or wrongCharge MC track candidates
3) **Hit efficiency**: Mean of all single hit efficiency of the MC track candidates labeled as found. The single hit efficiency is defined as the number of found hits divided by the number of all hits in a track. This information is encoded in the weight of the relations created by the MCRecoTracksMatcherModule.
4) **Clone rate**: Number of PR track candidates which are labeled clone divided by the number of PR track candidates which are labeled clone or matched (we do not distinguish clones with correct or wrong charge)
5) **Fake rate**: Number of PR track candidates which are labeled background or ghost divided by the total number of PR track candidates.


These definitions can be looked up in :code:`/tracking/scripts/validation/mc_side_module.py` and :code:`/tracking/scripts/validation/pr_side_module.py`.

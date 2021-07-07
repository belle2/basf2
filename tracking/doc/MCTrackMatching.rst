--------------
Track matching
--------------

This section describes the definition of the various status, that the matching of tracks can produce. 
The four main figures of merit for the track finder - the finding efficiency, the hit efficiency, the clone rate and the fake rate - are defined using these matching labels as described below.

~~~~~~~~~~~~~~~~~~~~~~~~~~
Overview: Available Status
~~~~~~~~~~~~~~~~~~~~~~~~~~

After running the TrackFinderMCTruthRecoTracksModule which creates Genfit Track Candidates from MC information (in the following called MC track candidates) 
and the "normal" track finder algorithm which uses hit information from the detector(in the following called PR track candidates), 
you can apply the MCRecoTracksMatcherModule, which creates relations between the two StoreArray s 
of track candidates by looking on the hit content. 
If the hit content of two track candidates has a non-zero intersection, 
a relation is created with the ration between the intersection number of hits to the total number of 
hits in the candidate as a weight (in both directions because the weight can be different as the total number 
of hits in a track can be different for MC and PR track candidates). 
The weights from PR to MC track candidates are called purity and from MC to PR track candidates efficiency. 
Only the single highest value for each PR and MC track candidates is stored in the relation array 
(so only the "best match" is stored) and only if the purity is above 2/3 and the efficiency is above 0.05.

After the matching, each PR and each MC track candidate is given a single label:

Tracks from Pattern Recognition can be,

*    matched,
*    clone, or
*    fake (= background or ghost)

as it can be seen in the PRToMCMatchInfo in ``TrackMatchLookUp.h``

Charged MCParticles can be

*    found or matched (we will call it found to not confuse with the PR track candidates)
*    merged or
*    missing.

as it can be seen in the MCToPRMatchInfo in ``TrackMatchLookUp.h``.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
When is a Track/MCParticle What?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We will first describe the labels here briefly 
(as it can also be found in the comments in the ``MCMatcherTracksModule.h``)
and then show some examples.

The PR track candidate can be classified into four categories, which are described in the following

* MATCHED

  * The highest efficiency PR track candidate of the highest purity  MC track candidate  to this PR track candidate  is the same as this PR track candidate. 
    This means the PR track candidate contains a high contribution of only one MC track candidate and is also the best of all PR track candidates 
    describing this MC track candidate.
* CLONE

  * The highest purity MC track candidate has a different highest efficiency PR track candidate than this track. 
    This means the PR track candidate contains high contributions of only one MC track candidate but a different other 
    PR track candidate contains an even higher contribution to this MC track candidate.

* BACKGROUND

  * The PR track candidate contains mostly hits, which are not part of any MC track candidate. 
    This normally means, that this PR track candidates is made of beam background hits or random combinations of hits. 
    Be careful: If e.g. only creating MC track candidates out of primary particles, 
    all found secondary particles will be called background (which is happening in the default validation)

* GHOST

  * The highest purity MC track candidate to this PR track candidate has a purity lower than the minimal purity given in 
    the parameter minimalPurity (2/3) or has an efficiency lower than the efficiency given in the parameter 
    minimalEfficiency (0.05). 
    This means that the PRTrack does not contain a significat number of a specific MCTrack nor can it considered only made of background.


MC track candidates are classified into three categories:

* MATCHED

  * The highest purity MC track candidate of the highest efficiency PR track candidate of this MC track candidate is the same as this MC track candidate. 
    This means the MC track candidate is well described by a PR track candidate and this 
    PR track candidate has only a significant contribution from this MC track candidate.

* MERGED

  * The highest purity MC track candidate of the highest efficiency PR track candidate of this MC track candidate is not the same as this MC track candidate. 
    This means this MC track candidate is mostly contained in a PR track candidate, 
    which in turn however better describes a MC track candidate different form this.

* MISSING
  * There is no highest efficiency PR track candidate to this MC track candidate, which also fulfills the minimal purity requirement.


Four examples are shown in the pictures. 
The circles on the left side shows the MC track candidates, the right side stands for the PR track candidates. 
The arrows depict that there are common hits, the percentage values shows the ratio.

 
.. figure:: /tracking/doc/figs/fom_found.png
  :width: 40em
  :align: center

  There is a one to one connection between a MCTrackCand and a track from the track finder. 
  The MCTrackCand is labeled found and the other track is labeled matched.

.. figure:: /tracking/doc/figs/fom_clone.png
  :width: 40em
  :align: center

  The MCTrackCand is found twice. The track from the track finder with the higher percentage 
  (the green one in this example) is labeled matched, the other one cloned. 
  The MCTrackCand is nevertheless labeled found.

.. figure:: /tracking/doc/figs/fom_fake.png
  :width: 40em
  :align: center

  The track from the track finder is created with hits from many different MCTrackCands. 
  As none of the corresponding hit ratios exceeds 66%, the track is called ghost or fake. 
  The hit ratios of the MCTrackCands itself do not play any role here.

.. figure:: /tracking/doc/figs/fom_background.png
  :width: 40em
  :align: center

  The found track does not describe any of the MCTrackCands well (or well enough) - but is made out of background hits. 
  This track is also called a fake or background.

~~~~~~~~~~~~~~~~
Figures of Merit
~~~~~~~~~~~~~~~~

The four main figures of merit, as also shown on the validation side, are:

* Finding efficiency: Number of MC track candidates which are labeled found divided by the total number of MC track candidates
* Hit efficiency: Mean of all single hit efficiency of the MC track candidates labeled as found. 
  The single hit efficiency is defined as the number of found hits divided by the number of all hits in a track. 
  This information is encoded in the weight of the relations created by the MCRecoTracksMatcherModule.
* Clone rate: Number of PR track candidates which are labeled clone divided by the number of PR 
  track candidates which are labeled clone or matched
* Fake rate: Number of PC track candidates which are labeled fake divided by the total number of PR track candidates.


These definitions can be looked up in :code:`/tracking/scripts/validation/mc_side_module.py` and :code:`/tracking/scripts/validation/pr_side_module.py`.

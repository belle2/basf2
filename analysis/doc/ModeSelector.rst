.. _ModeSelector:

ModeSelector
============

ModeSelector is a neural network classifier for hadronic FEI :math:`B` meson
tagging. The FEI assigns a signal probability (``sigProb``) to each :math:`B`
candidate independently, using only that candidate's own decay products.
ModeSelector instead uses the full set of FEI :math:`B` candidates in the event
simultaneously (all candidates with ``sigProb > 0.001``) to predict which
physics category the event belongs to and which specific FEI decay mode most
likely produced the tag :math:`B`.


When to use ModeSelector
------------------------

ModeSelector builds directly on the FEI: it was trained on FEI skims and requires them as input.
The selection on the ModeSelector output then supersedes any additional ``sigProb`` cut.

Its main strength is separating :math:`B^0` and :math:`B^+` events, which
substantially reduces crossfeed in analyses that do not fully reconstruct
the signal side.

The gain in performance depends on the signal mode: analyses with more inclusive signal sides
benefit the most, because the less constrained the signal reconstruction is, the more sensitive
it is to crossfeed from the wrong :math:`B` type.


Algorithm
---------

Two-stage network
~~~~~~~~~~~~~~~~~

ModeSelector runs two neural networks in sequence on each event.

.. code-block:: text

   All B candidates -> [feature matrix] -> category network -> B0 / B+ / continuum
                                                    |
                                                    v
                                             main network -> 139-class output -> BplusScore

The **category network** (3 outputs) classifies the event as
:math:`B^0`, :math:`B^+`, or continuum based on the full set of candidates.

The **main network** (139 outputs) then predicts which specific FEI decay mode
produced the tag :math:`B`. The 139 classes are:

- classes 0-135: signal modes, where the class index (``input_id``) directly encodes the :math:`B` type, decay mode index ``dmID``, and particle/antiparticle sign.
  Thus the 136 classes are (32 neutral FEI modes + 36 charged FEI modes) :math:`\times 2`.
- class 136: ``bad_tag`` (combinatorial background)
- class 137: ``cross_deltaC1`` (:math:`BB` crossfeed from the wrong :math:`B` charge)
- class 138: ``continuum``

Each ``input_id`` slot can in principle be occupied by multiple FEI
candidates (same :math:`B` type, decay mode, and charge). When this happens,
only the candidate with the highest ``sigProb`` is kept per slot
(deduplication). The input feature matrix has 9 blocks of per-candidate kinematic and
quality variables (FEI signal probability (:math:`B` and daughters), vertex fit quality (:math:`B` and daughters), D* veto mass differences, ``deltaE``, ``cosTBTO``)
across 136 deduplicated slots. To these, 12 event-level variables are
appended: 8 event-shape quantities (sphericity, thrust, thrust axis, aplanarity, Fox-Wolfram R2 + 3 harmonic moments) plus the total number of candidates, the ``input_id`` of the
highest-sigProb candidate, the highest-sigProb ``input_id`` from the other
:math:`B` type, and the experiment number. Sparse columns which do not encode 
information are excluded from the active network inputs. The main network additionally
receives the category network outputs and its predicted :math:`B` type as
inputs.


D* veto
~~~~~~~

The D* veto identifies FEI candidates where the :math:`B` was reconstructed
with a :math:`D` meson directly from the :math:`B` decay vertex, but the true
decay actually proceeded through a :math:`D^*` with a soft pion or :math:`\pi^0`
that was not reconstructed by the FEI.

These misidentified decays are a significant source of crossfeed: the missing
soft pion carries information about the :math:`B` flavour and charge, so its
absence leads to wrongly identified :math:`B` type.

Detection is based on the delta mass difference of a reconstructed
:math:`D^*` hypothesis:

.. math::

   \Delta m_{\mathrm{diff}} = m(D\pi) - m(D) - m^{\mathrm{PDG}}(D^* - D)

A small :math:`|\Delta m_{\mathrm{diff}}|` indicates a likely :math:`D^*`
decay. The D* veto produces two variables per :math:`B` candidate:

- ``Dstp_deltaMassDiff``: delta mass difference for the :math:`D^{*+}` hypothesis
- ``Dst0_deltaMassDiff``: delta mass difference for the :math:`D^{*0}` hypothesis

These are used as input features and are also stored as candidate ``ExtraInfo`` for offline selection.


Outputs
-------

The following variables are written by the module:

**Event-level** (``EventExtraInfo``):

``BplusScore``
  The main signed event-level score. The sign encodes the predicted
  :math:`B` type: positive for :math:`B^+`, negative for :math:`B^0`. The
  magnitude is the maximum main-network probability over candidates present in
  the predicted sector (background classes excluded). A score close to
  :math:`\pm 1` indicates high confidence; a score near 0 indicates a
  background-like event.

  On events where the predicted sector has no reconstructed candidate,
  a fallback score is used based on the sum of predicted-sector outputs which
  encode the predicted :math:`B` type.

``modeSelector_catBp``, ``modeSelector_catB0``, ``modeSelector_catCont``
  Category network probabilities for :math:`B^+`, :math:`B^0`, and continuum.
  The predicted sector is :math:`B^+` when
  ``modeSelector_catBp > modeSelector_catB0``.

**Candidate-level** (``ExtraInfo``):

``modeSelector_eqSigProb``
  The main-network probability assigned to a candidate's specific decay mode.
  Written only on candidates in the predicted sector (:math:`B^+` if
  ``BplusScore > 0``, :math:`B^0` if ``BplusScore < 0``). This can serve as
  a mode-aware complement to ``sigProb``.

``modeSelector_rank``
  Local sector ranking written on deduplicated representative candidates.
  In the predicted sector, candidates are ranked by ``modeSelector_eqSigProb``;
  in the non-predicted sector, by ``sigProb``. The two sectors are ranked
  independently.


Best candidate selection
------------------------

Because ModeSelector uses additional information with respect to FEI, its
best candidate (rank 1 by ``modeSelector_eqSigProb`` in the predicted sector)
can differ from the one selected by ``sigProb`` alone, though in practice the
two agree when the FEI already assigns a clearly dominant signal probability.
The mode probability from the main network uses the full event context rather
than a single candidate's features, and its training dataset better reflects
the composition expected in data.

``modeSelector_rank`` is written on all deduplicated candidates, in both
sectors. The ranking criterion differs by sector:

- **Predicted sector** (``modeSelector_rank == 1``): candidate with the
  highest ``modeSelector_eqSigProb``.
- **Non-predicted sector** (``modeSelector_rank == 1``): candidate with the
  highest ``sigProb``. Since ``modeSelector_eqSigProb`` is not defined for
  the non-predicted sector, only ``sigProb`` is used for ranking there.

The non-predicted sector contains candidates reconstructed as the :math:`B`
type opposite to what the category network predicts for the event. These
candidates form a crossfeed-enhanced sample that can serve as a
calibration or control sample. To restrict the analysis to the predicted
sector only, require ``BplusScore > 0`` for :math:`B^+` analyses or
``BplusScore < 0`` for :math:`B^0` analyses.


How to use
----------

After running the FEI, apply candidate preselections, build the rest of
event and continuum suppression, then add event shape variables and call
``modeSelector.modeSelector()``. The D* veto reconstruction and the neural
network evaluation are both set up by this single call.

The preselections ``Mbc > 5.23``, ``-0.15 < deltaE < 0.1``, and
``cosTBTO < 0.9`` were applied during training and are also used in the FEI
calibration. They must be reproduced at inference time::

    import modularAnalysis as ma
    import modeSelector

    track_mask = "[[dr < 2] and [abs(dz) < 4] and [pt > 0.2] and [thetaInCDCAcceptance==1]]"
    ecl_mask = ("[[[[clusterReg==1] and [E>0.080]] or [[clusterReg==2] and [E > 0.03]] "
                "or [[clusterReg==3] and [E > 0.06]]] and [clusterNHits > 1.5] "
                "and [abs(clusterTiming) < 200] and [thetaInCDCAcceptance==1]]")
    roe_mask = ("cleanMask", track_mask, ecl_mask)

    for b in ['B+:feiHadronic', 'B0:feiHadronic']:
        # Preselections must match the training setup and FEI calibration
        ma.applyCuts(b, '[Mbc > 5.23] and [-0.15 < deltaE < 0.1]', path=my_path)

        # Build rest of event and continuum suppression (required for cosTBTO)
        ma.buildRestOfEvent(b, path=my_path)
        ma.appendROEMasks(b, [roe_mask], path=my_path)
        ma.buildContinuumSuppression(b, 'cleanMask', path=my_path)

        ma.applyCuts(b, 'cosTBTO < 0.9', path=my_path)

    # Some event shape variables are required by ModeSelector
    ma.buildEventShape(
        allMoments=False,
        cleoCones=False,
        jets=False,
        collisionAxis=False,
        harmonicMoments=True,
        foxWolfram=True,
        sphericity=True,
        thrust=True,
        path=my_path,
    )

    # Add ModeSelector (D* veto reconstruction is included automatically)
    modeSelector.modeSelector(
        bp_list='B+:feiHadronic',
        b0_list='B0:feiHadronic',
        payload_cat_model='<cat_payload_name>',
        payload_main_model='<main_payload_name>',
        output_variable='BplusScore',
        path=my_path,
    )

    # Best candidate selection: keep the rank-1 candidate per list
    for b in ['B+:feiHadronic', 'B0:feiHadronic']:
        ma.applyCuts(b, 'extraInfo(modeSelector_rank) == 1', path=my_path)

Models are loaded from the conditions database using the payload names
specified via ``payload_cat_model`` and ``payload_main_model``, provided
in the recommended analysis global tag.

The following variables are available after running ``modeSelector()``.
Event-level outputs are in ``EventExtraInfo`` and accessed via
``eventExtraInfo(...)``:

- ``BplusScore``: main output score; positive for :math:`B^+`, negative for :math:`B^0`
- ``modeSelector_catBp``, ``modeSelector_catB0``, ``modeSelector_catCont``: category network outputs

Candidate-level outputs are in ``ExtraInfo`` and accessed via ``extraInfo(...)``:

- ``modeSelector_eqSigProb``: mode-aware (equalized) signal probability (predicted sector only)
- ``modeSelector_rank``: local sector candidate rank


Training
--------

Analysts do not normally need to retrain the networks. This section is
provided for reference.

Training is performed on run-dependent MC (MCrd) using FEI-skimmed events.
FEI calibration factors are applied as per-event weights during training,
so the network is conditioned on a sample composition that matches data. 
The training sample includes both :math:`BB` events and continuum, with
continuum downweighted to 25% relative importance compared to :math:`BB`.
MC truth matching is applied to assign labels, making use of 
``mostcommonBTagPDG`` and ``mostcommonBTagDeltaP``.

The category network is trained first. The main network is trained afterwards,
receiving both the feature matrix and the category network output as inputs,
so the two stages are coupled. The architecture is fully connected with ReLU
activations.

All training scripts and configuration are in ``analysis/scripts/modeSelector/``:

- ``train.py``: training pipeline for both networks
- ``config.py``: training configuration, feature definitions and used FEI calibration weights
- ``convert_to_onnx.py``: export trained models to basf2 MVA weightfiles using ONNX

See ``analysis/scripts/modeSelector/README.md`` for full details.


Functions
---------

.. currentmodule:: modeSelector

.. autofunction:: modeSelector

.. autofunction:: addDstarVeto

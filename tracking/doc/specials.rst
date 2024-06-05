
Tracking for Special Classes of Tracks
--------------------------------------

.. _tracking_v0Finding:

V0 Finding
""""""""""
V0s are neutral particles we reconstruct from their decay into two charged tracks, such as :math:`K_S\to\pi^+\pi^-` or :math:`\Lambda\to p\pi^-`. Due to their relatively long lifetime, they mostly decay outside of the beam pipe. By default, the track parameters extrapolated to the point of closest approach (POCA) to IP (0, 0, 0) are stored in the mDST. This extrapolation also includes the correction for potential material effects and energy losses. Because the daughters of V0s are not produced at the POCA, they need some special treatment. The `V0Finder` module takes care of this.

Also photon conversions (:math:`\gamma\to e^+e^-` inside material) need the same kind of special treatment for the same reasons, therefore the `V0Finder` takes care of these as well.

The `V0Finder` takes care specifically of V0s with a decay vertex outside of the beam pipe (i.e. transverse distance from origin above 1 cm). V0s that decay inside the beam pipe can be reconstructed at analysis level using the standard reconstruction procedure (i.e. with `reconstructDecay` plus vertex fit), and are therefore ignored by the `V0Finder`.

The `V0Finder` is run during reconstruction (i.e. raw data processing for mDST production) so that it can access all the information it needs (hits attached to tracks, magnetic field map, detector geometry), and that are not available when running analyses. It performs the following steps.

* Combination: consider each pair made of one positive and one negative track.
* Preselection: compute the invariant mass of the two tracks.

  * This requires knowledge of the angle between the momenta

    .. math::

        m^2 = (E_++E_-)^2 - (\vec p_+ + \vec p_-)^2 = (E_++E_-)^2 - p_+^2 - p_-^2 - 2p_+p_-\cos\alpha

    which is unavailable because the vertex position is not yet known; therefore a range of possible invariant masses is computed (with the minimum obtained assuming :math:`\cos\alpha=1` and the maximum assuming :math:`\cos\alpha=-1`), and this range is required to overlap with an invariant mass window (see ``massRangeKshort`` and ``massRangeLambda`` module parameters)
  * This cut is not applied to photon conversions.
  * Since release-08, an additional cut is applied to reject pairs of tracks that definitely intersect within the beam pipe, and thus must not be handled by the `V0Finder`. This is meant to speed up the module by skipping some pairs without performing the vertex fitting (see below). The tracks are approximated with straight lines (that pass by the POCA to the IP and have the same direction as the momentum at the POCA to the IP), and the point of closest approach between the two lines is found analytically. If the distance of such point from the IP is smaller than ``precutRho`` (0.5 cm by default), the pair is rejected. Also, pairs of almost-parallel tracks (:math:`\cos\alpha` > ``precutCosAlpha``, 0.9 by default) are always accepted, as the straight-line approximation for the vertex is less reliable in this case.

* Vertex fitting: the vertex position is obtained from a so-called vertex fit. The `V0Finder` employs the RAVE fitter from the GenFit package and exploits the knowledge about first hit position, energy losses of the tracks inside detector material, magnetic field non-uniformities; if the fit does not converge (:math:`\chi^2/NDF > ` ``vertexChi2CutOutside`` module parameter), or the vertex is found to be inside the beam pipe, the candidate is rejected.
* Inner hits removal: if the tracks were produced at the fitted vertex, they can not have left any hit in the helix segments that come before the vertex; if any such hit is attached, it must be wrong and can bias the track fit, therefore they are removed, then the tracks are refitted, and the vertex is fitted again; the check for inner hits is also repeated.

  * If the fit with the refitted tracks fails, the previous result is kept.
  * This step can be skipped with the ``v0FitterMode`` module parameter

* Selection: now that the vertex is fitted, the invariant mass can be computed and a cut applied to it (see ``invMassRangeKshort``, ``invMassRangeLambda`` and ``invMassRangePhoton`` module parameters)

Candidates that pass the selection are stored to V0 objects, which contain the reference (array index) to two ``Track``\s and two ``TrackFitResult``\s with the parameters of the helices at the decay vertex position.

.. note::

   The ``TrackFitResult``\s associated to the ``Track``\s normally store the helix parameters at the perigee (point of closest approach to the IP), but these might be different from the ones at the decay vertex position due to energy losses, magnetic field non-uniformities and material effects. In order to reconstruct the V0 vertex correctly, the parameters at the decay vertex must be used (this is particularly important when computing the angle between the tracks).

During analysis, V0 lists are loaded using functions such as `stdKshorts` and `stdLambdas`. What these functions do is

* Take candidates from V0 objects

  * Make candidates using the ``TrackFitResult``\s associated to the V0 object for the daughters
  * Fit their vertices (using TreeFit or KFit)
  * Apply a cut on the invariant mass

* Reconstruct V0s that decayed inside the beam pipe

  * Use `reconstructDecay` to make candidates with a loose invariant mass cut (if the V0 decayed inside the beam pipe, the error we make on the invariant mass because we don't know the vertex position yet is small)
  * Fit their vertices (using TreeFit or KFit)
  * Apply a cut on the invariant mass

* Merge the two candidates lists, keeping only the candidate from a V0 object in case of duplicates

.. b2-modules::
   :package: tracking
   :modules: V0Finder
   :io-plots:

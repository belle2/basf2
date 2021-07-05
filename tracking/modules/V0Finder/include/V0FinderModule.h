/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
//Object with performing the actual algorithm:
#include <tracking/v0Finding/fitter/V0Fitter.h>

#include <mdst/dataobjects/Track.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <string>
#include <memory>

namespace Belle2 {
  /** A V0 finder module.
   *
   *  Pairs up all positive and negative tracks,
   *  tries to find vertices between them,
   *  stores found vertices.
   *  Only vertices outside the beam pipe are saved, the others are
   *  recovered in a following analysis level
   *  for use inside the beam pipe, one outside (the difference being
   *
   *  Cut outside the beam pipe is maximum chi^2.
   *  The value used as beam pipe radius is an option.
   *
   *  FIXME once particle hypotheses are thought through, we should
   *  also deal with photon conversion.
   *
   *  The resulting pairs of tracks are stored as mdst::V0.
   */
  class V0FinderModule : public Module {
  public:
    /** Setting of module description, parameters. */
    V0FinderModule();

    /** Acknowledgement of destructor. */
    ~V0FinderModule() override = default;

    /** Registration of StoreArrays, Relations, check proper GenFit setup. */
    void initialize() override;

    /** Creates Belle2::V0s from Belle2::Tracks as described in the class documentation. */
    void event() override;

  private:

    std::string m_arrayNameTrack;     ///< StoreArray name of the Tracks          (Input).
    StoreArray<Track> m_tracks;       ///< Actually array of mdst Tracks.

    std::unique_ptr<V0Fitter> m_v0Fitter;  ///< Object containing the actual algorithm.
    std::string m_arrayNameRecoTrack;      ///< StoreArray name of the RecoTracks      (Input).
    std::string m_arrayNameCopiedRecoTrack;///< StoreArray name of the RecoTracks
    std::string m_arrayNameTFResult;       ///< StoreArray name of the TrackFitResults (In- and Output).
    std::string m_arrayNameV0;             ///< StoreArray name of the V0              (Output).

    double m_beamPipeRadius;          ///< Radius where inside/outside beampipe is defined.
    double m_vertexChi2CutOutside;    ///< Chi2 cut for V0s outside of the beampipe. Applies to all.
    int    m_v0FitterMode;            ///< fitter mode (0: store V0 at the first vertex fit, regardless of inner hits, 1: remove hits inside the V0 vertex position, 2: mode 1 +  don't use SVD hits if there is only one available SVD hit-pair (default))

    /// range for reconstructed Kshort mass used after removing material effects and inner hits
    /// default range set to { 0.425, 0.575 } GeV
    std::tuple<double, double> m_invMassRangeKshort = { 0.425, 0.575 };
    /// range for reconstructed Lambda mass used after removing material effects and inner hits
    /// default range set to { 1.09, 1.14 } GeV
    std::tuple<double, double> m_invMassRangeLambda = { 1.09, 1.14 };
    /// range for reconstructed Photon mass used after removing material effects and inner hits
    /// default range set to { 0, 0.1 } GeV
    std::tuple<double, double> m_invMassRangePhoton = { 0, 0.1 };

    bool m_validation;                         ///< Flag if use validation.
    std::string m_arrayNameV0ValidationVertex; ///< StoreArray name of the V0ValidationVertex.

    /// range for reconstructed Kshort mass used for pre-selection
    /// default range set to nominal KS mass + 0.015 GeV and 0.45 GeV
    std::tuple<double, double> m_preFilterMassRangeKshort = { 0.45, Const::K0Mass + 0.015 };
    /// range for reconstructed Lambda mass used for pre-selection
    /// Default range set to nominal Lambda mass +/- 0.03
    std::tuple<double, double> m_preFilterMassRangeLambda = { Const::Lambda.getMass() - 0.03, Const::Lambda.getMass() + 0.03 };

    /** helper function that gets the approximate mass range for the two given tracks and rejects candidates which
      do not fall into a user given mass range for lambda and Kshort.
      @param trackPlus: the track for the positively charged candidate
      @param trackMinus: the track for the negatively charged candidate
      @param v0Hypothesis: the hypothesis for the V0 (Lambda, or Kshort, for all others nothing happens)
    */
    bool preFilterTracks(const Track* trackPlus, const Track* trackMinus, const Const::ParticleType& v0Hypothesis);
    // buffer some variables to speed up time, actual values will be calculated at initialization
    double m_mKshortMin2 = 0; ///< pre-calculated mininum Kshort mass squared
    double m_mKshortMax2 = 0; ///< pre-calculated maximum Kshort mass squared
    double m_mLambdaMin2 = 0; ///< pre-calculated mininum Lambda mass squared
    double m_mLambdaMax2 = 0; ///< pre-calculated maximum Lambda mass squared
  };
}

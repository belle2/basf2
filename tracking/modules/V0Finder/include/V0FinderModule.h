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
#include <tracking/v0Finding/fitter/NewV0Fitter.h>

#include <mdst/dataobjects/Track.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <string>
#include <memory>

namespace Belle2 {

  /**
   * V0 finder module.
   *
   * Pairs up all positive and negative tracks,
   * tries to find vertices between them,
   * stores found vertices.
   * Only vertices outside the beam pipe are saved, the others are
   * recovered in a following analysis level.
   *
   * Cut outside the beam pipe is maximum chi^2.
   * The value used as beam pipe radius is an option.
   *
   * The resulting pairs of tracks are stored as mdst::V0.
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

    /** Prints status summary. */
    void terminate() override;

  private:

    std::string m_arrayNameTrack;     ///< StoreArray name of the Tracks          (Input).
    StoreArray<Track> m_tracks;       ///< Actually array of mdst Tracks.

    std::unique_ptr<V0Fitter> m_v0Fitter;  ///< Object containing the actual algorithm.
    std::unique_ptr<NewV0Fitter> m_newV0Fitter;  ///< Object containing the actual algorithm.
    std::string m_arrayNameRecoTrack;      ///< StoreArray name of the RecoTracks      (Input).
    std::string m_arrayNameCopiedRecoTrack;///< StoreArray name of the RecoTracks
    std::string m_arrayNameTFResult;       ///< StoreArray name of the TrackFitResults (In- and Output).
    std::string m_arrayNameV0;             ///< StoreArray name of the V0              (Output).

    double m_beamPipeRadius;          ///< Radius where inside/outside beampipe is defined.
    double m_vertexChi2CutOutside;    ///< Chi2 cut for V0s outside of the beampipe. Applies to all.
    int    m_v0FitterMode;            ///< fitter mode
    double m_precutRho;      ///< preselection cut on transverse radius of the track pair POCA
    double m_precutCosAlpha; ///< preselection cut on opening angle of the track pair
    bool   m_useNewV0Fitter; ///< toggle between old (false) and new (true) V0 fitter
    bool   m_validation;     ///< true to use validation.

    /// range for reconstructed Kshort mass used after removing material effects and inner hits
    std::tuple<double, double> m_invMassRangeKshort = { 0.425, 0.575 };
    /// range for reconstructed Lambda mass used after removing material effects and inner hits
    std::tuple<double, double> m_invMassRangeLambda = { 1.09, 1.14 };
    /// range for reconstructed Photon mass used after removing material effects and inner hits
    std::tuple<double, double> m_invMassRangePhoton = { 0, 0.1 };

    std::string m_arrayNameV0ValidationVertex; ///< StoreArray name of the V0ValidationVertex.

    /// range for reconstructed Kshort mass used for pre-selection
    std::tuple<double, double> m_preFilterMassRangeKshort = { 0.45, Const::K0Mass + 0.015 };
    /// range for reconstructed Lambda mass used for pre-selection
    std::tuple<double, double> m_preFilterMassRangeLambda = { Const::Lambda.getMass() - 0.03, Const::Lambda.getMass() + 0.03 };

    /**
     * Track pair preselection based on a point-of-closest-approach of two tracks.
     * Tracks are approximated with staright lines using their perigee parameters.
     * @param trk1 first track
     * @param trk2 second track
     * @return true, if preselection criteria are satisfied.
     */
    bool isTrackPairSelected(const Track* trk1, const Track* trk2);

    /**
     * Helper function that gets the approximate mass range for the two given tracks and rejects candidates which
     * do not fall into a user given mass range for lambda and Kshort.
     * @param trackPlus positively charged track
     * @param trackMinus negatively charged track
     * @param v0Hypothesis hypothesis for the V0 (Lambda, or Kshort, for all others nothing happens)
     * @return true if selected
     */
    bool preFilterTracks(const Track* trackPlus, const Track* trackMinus, const Const::ParticleType& v0Hypothesis);

    /**
     * V0 fitting and storing
     * @param trackPlus positively charged track
     * @param trackMinus negatively charged track
     * @param v0Hypothesis hypothesis for the V0
     */
    void fitAndStore(const Track* trackPlus, const Track* trackMinus, const Const::ParticleType& v0Hypothesis);

    // buffer some variables to speed up time, actual values will be calculated at initialization
    double m_mKshortMin2 = 0; ///< pre-calculated mininum Kshort mass squared
    double m_mKshortMax2 = 0; ///< pre-calculated maximum Kshort mass squared
    double m_mLambdaMin2 = 0; ///< pre-calculated mininum Lambda mass squared
    double m_mLambdaMax2 = 0; ///< pre-calculated maximum Lambda mass squared

    // counter for V0Finder statistics
    int m_allStored = 0;    ///< counter for all saved V0s
    int m_nForceStored = 0; ///< counter for saved V0s failing to remove the inner hits
    int m_nHitRemoved  = 0; ///< counter for saved V0s successfully removing the inner hits
  };
}

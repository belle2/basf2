#pragma once

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/V0.h>
#include <tracking/dataobjects/V0ValidationVertex.h>

#include <TVector3.h>

#include <utility>

namespace genfit {
  class MeasuredStateOnPlane;
  class GFRaveVertex;
  class Track;
}

namespace Belle2 {

  /** V0Fitter class to create V0 mdst's from reconstructed tracks.
   * To use this class, give the V0Fitter a positive and a negative charged track and call the fitAndStore function.
   *
   * v0Fitter.fitAndStore(B2TrackPositive, B2TrackNegative, V0HypothesisParticleType);
   */
  class V0Fitter {
    friend class V0FitterTest_GetTrackHypotheses_Test;
    friend class V0FitterTest_EnableValidation_Test;
    friend class V0FitterTest_InitializeCuts_Test;

  public:
    /// Constructor for the V0Fitter.
    V0Fitter(const std::string& trackFitResultColName = "", const std::string& v0ColName = "",
             const std::string& v0ValidationVertexColName = "",
             const std::string& gfTrackColName = "");

    /// Enable validation output.
    void enableValidation() { m_validation = true; }

    /// Initialize the cuts which will be applied during the fit and store process.
    void initializeCuts(double beamPipeRadius,
                        double vertexChi2CutOutside);

    /// Fit V0 with given hypothesis and store if fit was successful.
    bool fitAndStore(const Track* trackPlus, const Track* trackMinus, const Const::ParticleType& v0Hypothesis);

  private:

    /** Starting point: point closest to axis where either track is defined
     * This is intended to reject tracks that curl away before
     * meeting, there are corner cases where this could throw away
     * legitimate candidates, namely where one track makes a full
     * circle through the detector without hitting any detectors
     * then making it past Rstart without hitting the detector there
     * -- while still being part of the V0.  Unlikely, I didn't find
     * a single example in MC.  On the other hand it rejects
     * impossible candidates.
     * @param stPlus
     * @param stMinus
     * @return
     */
    bool rejectCandidate(genfit::MeasuredStateOnPlane& stPlus, genfit::MeasuredStateOnPlane& stMinus);

    /** Fit the V0 vertex.
     *
     * @param trackPlus
     * @param trackMinus
     * @param vertex Result of the fit is returned via reference.
     * @return
     */
    bool fitVertex(genfit::Track& trackPlus, genfit::Track& trackMinus, genfit::GFRaveVertex& vertex);

    /// Extrapolate the fit results to the perigee to the vertex.
    bool extrapolateToVertex(genfit::MeasuredStateOnPlane& stPlus, genfit::MeasuredStateOnPlane& stMinus,
                             const TVector3& vertexPosition);

    /// Getter for magnetic field in z direction at the vertex position.
    double getBzAtVertex(const TVector3& vertexPosition);

    /// Build TrackFitResult of V0 Track and set relation to genfit Track.
    TrackFitResult* buildTrackFitResult(const genfit::Track& track, const genfit::MeasuredStateOnPlane& msop, const double Bz,
                                        const Const::ParticleType& trackHypothesis);

    /// Get track hypotheses for a given v0 hypothesis.
    std::pair<Const::ParticleType, Const::ParticleType> getTrackHypotheses(const Const::ParticleType& v0Hypothesis);

  private:
    bool m_validation;  ///< Validation flag.
    std::string m_RecoTrackColName;   ///< RecoTrackColName (input).

    StoreArray<TrackFitResult> m_trackFitResults;  ///< TrackFitResultColName (output).
    StoreArray<V0> m_v0s;  ///< V0ColName (output).
    StoreArray<V0ValidationVertex> m_validationV0s;  ///< V0ValidationVertexColName (output, optional).

    double m_beamPipeRadius;  ///< Radius where inside/outside beampipe is defined.
    double m_vertexChi2CutOutside;  ///< Chi2 cut outside beampipe.
  };

}


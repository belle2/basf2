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

  class V0Fitter {
    friend class V0FitterTest_GetTrackHypotheses_Test;
    friend class V0FitterTest_EnableValidation_Test;
    friend class V0FitterTest_InitializeCuts_Test;

  public:
    V0Fitter(const std::string& trackFitResultColName = "", const std::string& v0ColName = "",
             const std::string& v0ValidationVertexColName = "",
             const std::string& gfTrackColName = "");

    void enableValidation() { m_validation = true; }

    void initializeCuts(double beamPipeRadius,
                        double vertexChi2CutInside,
                        double massWindowKshortInside,
                        double vertexChi2CutOutside);

    bool fitAndStore(const Track* trackPlus, const Track* trackMinus, const Const::ParticleType& v0Hypothesis);

  private:
    bool rejectCandidate(genfit::MeasuredStateOnPlane& stPlus, genfit::MeasuredStateOnPlane& stMinus);

    bool fitVertex(genfit::Track* trackPlus, genfit::Track* trackMinus, genfit::GFRaveVertex& vertex);

    bool extrapolateToVertex(genfit::MeasuredStateOnPlane& stPlus, genfit::MeasuredStateOnPlane& stMinus,
                             const TVector3& vertexPosition);

    double getBzAtVertex(const TVector3& vertexPosition);

    TrackFitResult* buildTrackFitResult(genfit::Track* track, genfit::MeasuredStateOnPlane& msop, double Bz,
                                        const Const::ParticleType& trackHypothesis);

    std::pair<Const::ParticleType, Const::ParticleType> getTrackHypotheses(const Const::ParticleType& v0Hypothesis);

  private:
    bool m_validation;
    std::string m_GFTrackColName;

    StoreArray<TrackFitResult> m_trackFitResults;
    StoreArray<V0> m_v0s;
    StoreArray<V0ValidationVertex> m_validationV0s;

    double m_beamPipeRadius;
    double m_vertexChi2CutInside;
    double m_massWindowKshortInside;
    double m_vertexChi2CutOutside;
  };

}


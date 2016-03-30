#pragma once

#include <mdst/dataobjects/V0.h>

#include <TVector3.h>

namespace Belle2 {

  /**
   * Class which stores some additional information on V0 vertices.
   * The information is created by RAVE and usually lost when the V0 is stored.
   * For some studies, this information is however useful.
   */
  class V0ValidationVertex : public V0 {
  public:
    V0ValidationVertex() :
      V0(),
      m_fittedVertexPosition(TVector3(0, 0, 0)),
      m_fittedMomentum(0),
      m_fittedInvariantMass(0),
      m_chi2(0)
    {}

    V0ValidationVertex(const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairPositive,
                       const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairNegative) :
      V0(trackPairPositive, trackPairNegative),
      m_fittedVertexPosition(TVector3(0, 0, 0)),
      m_fittedMomentum(0),
      m_fittedInvariantMass(0),
      m_chi2(0)
    {}

    V0ValidationVertex(const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairPositive,
                       const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairNegative,
                       const TVector3& fittedVertexPosition,
                       const TMatrixDSym& fittedVertexPositionCovariance,
                       const double& momentum,
                       const double& invariantMass,
                       const double& chi2) :
      V0(trackPairPositive, trackPairNegative),
      m_fittedVertexPosition(fittedVertexPosition),
      m_fittedVertexPositionCovariance(fittedVertexPositionCovariance),
      m_fittedMomentum(momentum),
      m_fittedInvariantMass(invariantMass),
      m_chi2(chi2)
    {}

    TVector3 getVertexPosition() const { return m_fittedVertexPosition; }

    TMatrixDSym getVertexPositionCovariance() const { return m_fittedVertexPositionCovariance; }

    float getFittedMomentum() const { return m_fittedMomentum; }

    float getFittedInvariantMass() const { return m_fittedInvariantMass; }

    float getVertexChi2() const { return m_chi2; }

  private:
    TVector3 m_fittedVertexPosition;
    TMatrixDSym m_fittedVertexPositionCovariance;
    float m_fittedMomentum;
    float m_fittedInvariantMass;
    float m_chi2;

    ClassDef(V0ValidationVertex, 1);
  };

}

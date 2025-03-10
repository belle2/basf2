/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <mdst/dataobjects/V0.h>

#include <Math/Vector3D.h>

namespace Belle2 {

  /**
   * Class which stores some additional information on V0 vertices.
   * The information is created by RAVE and lost when the V0 is stored.
   * For a reconstruction level validation, this additional information has to be stored. To enable the usage of this class, use the validation flag of the V0FinderModule.
   */
  class V0ValidationVertex : public V0 {
  public:

    /** Default Constructor. */
    V0ValidationVertex() :
      V0(),
      m_fittedMomentum(0),
      m_fittedInvariantMass(0),
      m_chi2(0)
    {}

    /** Constructor equivalent to the constructor of the V0 mdst dataobjects.
     *
     * @param trackPairPositive Positive charged Belle2::Track and TrackFitResult from the V0.
     * @param trackPairNegative Negative charged Belle2::Track and TrackFitResult from the V0.
     * @param vertexX The x coordinate of the fitted vertex position.
     * @param vertexY The y coordinate of the fitted vertex position.
     * @param vertexZ The z coordinate of the fitted vertex position.
     */
    V0ValidationVertex(const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairPositive,
                       const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairNegative,
                       double vertexX, double vertexY, double vertexZ) :
      V0(trackPairPositive, trackPairNegative, vertexX, vertexY, vertexZ),
      m_fittedMomentum(0),
      m_fittedInvariantMass(0),
      m_chi2(0)
    {}

    /** Constructor to initialize a V0 with additional information which is only available on reconstruction level.
     *
     * @param trackPairPositive Positive charged Belle2::Track and TrackFitResult from the V0.
     * @param trackPairNegative Negative charged Belle2::Track and TrackFitResult from the V0.
     * @param fittedVertexPosition Fitted vertex position from RAVE.
     * @param fittedVertexPositionCovariance Covariance of the vertex position from RAVE.
     * @param momentum Reconstructed momentum from RAVE.
     * @param invariantMass Reconstructed invariant mass from RAVE.
     * @param chi2 Chi2 of vertex fit from RAVE.
     */
    V0ValidationVertex(const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairPositive,
                       const std::pair<const Belle2::Track*, const Belle2::TrackFitResult*>& trackPairNegative,
                       const ROOT::Math::XYZVector& fittedVertexPosition,
                       const TMatrixDSym& fittedVertexPositionCovariance,
                       const double& momentum,
                       const double& invariantMass,
                       const double& chi2) :
      V0(trackPairPositive, trackPairNegative, fittedVertexPosition.X(), fittedVertexPosition.Y(), fittedVertexPosition.Z()),
      m_fittedVertexPositionCovariance(fittedVertexPositionCovariance),
      m_fittedMomentum(momentum),
      m_fittedInvariantMass(invariantMass),
      m_chi2(chi2)
    {}

    /// Getter for the reconstructed vertex position. Aliases getFittedVertexPosition for backwards compatibility.
    ROOT::Math::XYZVector getVertexPosition() const { return getFittedVertexPosition(); }

    /// Getter for the reconstructed vertex position covariance.
    TMatrixDSym getVertexPositionCovariance() const { return m_fittedVertexPositionCovariance; }

    /// Getter for the reconstructed momentum.
    float getFittedMomentum() const { return m_fittedMomentum; }

    /// Getter for the reconstructed invariant mass.
    float getFittedInvariantMass() const { return m_fittedInvariantMass; }

    /// Getter for the chi2 of the vertex fit.
    float getVertexChi2() const { return m_chi2; }

  private:
    TMatrixDSym m_fittedVertexPositionCovariance;  ///< vertex position covariance
    float m_fittedMomentum; ///< reconstructed momentum
    float m_fittedInvariantMass; ///< reconstructed invariant mass
    float m_chi2; ///< reconstructed chi2

    ClassDef(V0ValidationVertex, 4);  ///< Dataobjects for the reconstruction level validation of V0 reconstruction.
  };

}

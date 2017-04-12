/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

#include <genfit/Tools.h>

namespace Belle2 {
  class CKFCDCToVXDStateObject {
  public:
    static constexpr unsigned int N = 7;

    using SeedObject = RecoTrack;
    using HitObject = SpacePoint;

    void initialize(RecoTrack* seed)
    {
      m_seedRecoTrack = seed;
      m_measuredStateOnPlane = seed->getMeasuredStateOnPlaneFromFirstHit();
      m_chi2 = m_seedRecoTrack->getTrackFitStatus()->getChi2();
    }

    std::pair<RecoTrack*, std::vector<const SpacePoint*>> finalize() const
    {
      std::vector<const SpacePoint*> spacePoints;
      spacePoints.reserve(N);

      const auto& spacePointAdder = [&spacePoints](const CKFCDCToVXDStateObject * walkObject) {
        const SpacePoint* spacePoint = walkObject->getSpacePoint();
        if (spacePoint) {
          spacePoints.push_back(spacePoint);
        }
      };
      walk(spacePointAdder);

      return std::make_pair(getSeedRecoTrack(), spacePoints);
    }

    void buildFrom(const CKFCDCToVXDStateObject* parent, const SpacePoint* spacePoint)
    {
      m_parent = parent;
      m_seedRecoTrack = parent->getSeedRecoTrack();
      m_lastLayer = parent->getLastLayer() - 1;
      m_spacePoint = spacePoint;
      m_measuredStateOnPlane = parent->getMeasuredStateOnPlane();
    }

    void advance()
    {
      // TODO: Rewrite this with Eigen!
      // TODO: Test this!
      if (m_spacePoint) {
        const std::vector<genfit::PlanarMeasurement> measurements = m_spacePoint->getGenfitCompatible();
        B2ASSERT("Must have exactly 2 measurements", measurements.size() == 2);

        // Extrapolate from k-1 -> k. The mSoP is copied from the state before, so it is k-1.
        // By extrapolating, we will will go to k.
        const genfit::SharedPlanePtr& plane = measurements.front().constructPlane(m_measuredStateOnPlane);
        m_measuredStateOnPlane.extrapolateToPlane(plane);

        // We will change the state x_k, the covariance C_k and the chi2
        TVectorD x_k_old = m_measuredStateOnPlane.getState();
        TMatrixDSym C_k_old = m_measuredStateOnPlane.getCov();

        TVectorD& x_k_new = m_measuredStateOnPlane.getState();
        TMatrixDSym& C_k_new = m_measuredStateOnPlane.getCov();

        // Loop over the two clusters and extract the change for x_k and C_k.
        for (const auto& clusterMeasurement : measurements) {
          const std::vector<genfit::MeasurementOnPlane*> measurementsOnPlane = clusterMeasurement.constructMeasurementsOnPlane(
                m_measuredStateOnPlane);
          B2ASSERT("There should be exactly one measurement on plane", measurementsOnPlane.size() == 1);
          const genfit::MeasurementOnPlane& measurementOnPlane = *(measurementsOnPlane.front());

          const TVectorD& m_k = measurementOnPlane.getState();
          const TMatrixD& H_k = measurementOnPlane.getHMatrix()->getMatrix();
          const TMatrixD& H_k_t = TMatrixD(TMatrixD::kTransposed, H_k);
          const TMatrixDSym& V_k = measurementOnPlane.getCov();

          TMatrixDSym partOfK_k = (V_k + C_k_old.Similarity(H_k));
          genfit::tools::invertMatrix(partOfK_k);
          const TMatrixD& K_k = C_k_old * H_k_t * partOfK_k;

          C_k_new -= partOfK_k.Similarity(C_k_old * H_k);
          x_k_new += K_k * (m_k - H_k * x_k_old);

          const TVectorD& residual = m_k - H_k * x_k_new;
          TMatrixDSym someMatrix = V_k - C_k_new.Similarity(H_k);
          genfit::tools::invertMatrix(someMatrix);

          m_chi2 += someMatrix.Similarity(residual);
        }
      }
    }

    // Getters
    const CKFCDCToVXDStateObject* getParent() const
    {
      return m_parent;
    }

    RecoTrack* getSeedRecoTrack() const
    {
      return m_seedRecoTrack;
    }

    const genfit::MeasuredStateOnPlane& getMeasuredStateOnPlane() const
    {
      return m_measuredStateOnPlane;
    }

    const SpacePoint* getSpacePoint() const
    {
      return m_spacePoint;
    }

    unsigned int getLastLayer() const
    {
      return m_lastLayer;
    }

    unsigned int getNumberOfHoles() const
    {
      unsigned int numberOfHoles = 0;

      walk([&numberOfHoles](const CKFCDCToVXDStateObject * walkObject) {
        if (not walkObject->getSpacePoint()) {
          numberOfHoles++;
        }
      });

      return numberOfHoles;
    }

  private:
    RecoTrack* m_seedRecoTrack = nullptr;
    const SpacePoint* m_spacePoint = nullptr;
    unsigned int m_lastLayer = N;
    const CKFCDCToVXDStateObject* m_parent = nullptr;
    genfit::MeasuredStateOnPlane m_measuredStateOnPlane;
    double m_chi2 = 0;

    void walk(const std::function<void(const CKFCDCToVXDStateObject*)> f) const
    {
      const CKFCDCToVXDStateObject* walkObject = this;

      while (walkObject != nullptr) {
        f(walkObject);
        walkObject = walkObject->getParent();
      }
    }
  };
}
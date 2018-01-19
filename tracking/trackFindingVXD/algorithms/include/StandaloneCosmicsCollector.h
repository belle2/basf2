/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Metzner                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>
#include <utility>
#include <Eigen/Dense>
#include <Eigen/Geometry>

#include <framework/datastore/StoreArray.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

namespace Belle2 {
  class StandaloneCosmicsCollector {

  public:
    /// Constructor
    StandaloneCosmicsCollector() = default;

    /// Destructor
    ~StandaloneCosmicsCollector() = default;


    void addSpacePoints(StoreArray<SpacePoint>& SPs)
    {
      m_spacePoints.clear();
      m_direction.clear();
      m_start.clear();
      m_chi2 = 10;
      for (auto& sp : SPs) {
        addSpacePoint(&sp);
      }
    }


    void addSpacePoint(const SpacePoint* SP)
    {
      auto forwardIt = std::lower_bound(m_spacePoints.begin(), m_spacePoints.end(), SP,
                                        [this](const SpacePoint * lhs, const SpacePoint * rhs) -> bool { return this->compareRads(lhs, rhs); });
      m_spacePoints.insert(forwardIt, SP);
    }


    bool doFit(double qualityCut, int maxRejected, int minSPs)
    {
      bool fitting = true;
      int rejected = 0;

      while (m_chi2 > qualityCut && fitting) {
        fitting = doLineFit(minSPs);
        if (not fitting) {return fitting;}
        if (m_chi2 > qualityCut) {
          B2WARNING("Refitting without sp with index " << m_shittiest.second << " and chi2 contribution " << m_shittiest.first << "...");
          m_spacePoints.erase(m_spacePoints.begin() + m_shittiest.second);
          rejected++;
        }
        if (rejected > maxRejected) { B2WARNING("Rejected " << rejected << "!"); return false; }
      }
      return fitting;
    }

    std::pair<std::vector<double>, std::vector<double>> getResult()
    {
      return std::pair<std::vector<double>, std::vector<double>> (m_start, m_direction);
    }

    std::vector<const SpacePoint*> getSPTC()
    {
      return m_spacePoints;
    }

    double getChi2()
    {
      return m_chi2;
    }


  private:
    bool doLineFit(int minSPs)
    {
      int nHits = m_spacePoints.size();
      B2WARNING("Trying fit with " << nHits << " hits...");
      if (nHits < minSPs) { B2WARNING("Only " << nHits << " hits!"); return false; };

      Eigen::Matrix<double, 3, 1> average = Eigen::Matrix<double, 3, 1>::Zero(3, 1);
      Eigen::Matrix<double, Eigen::Dynamic, 3> data = Eigen::Matrix<double, Eigen::Dynamic, 3>::Zero(nHits, 3);
      Eigen::Matrix<double, Eigen::Dynamic, 3> P = Eigen::Matrix<double, Eigen::Dynamic, 3>::Zero(nHits, 3);

      for (const auto& sp : m_spacePoints) {
        average(0) += sp->getPosition().X();
        average(1) += sp->getPosition().Y();
        average(2) += sp->getPosition().Z();
      }
      average *= 1. / nHits;

      int index = 0;
      for (const auto& sp : m_spacePoints) {
        data(index, 0) = sp->getPosition().X();
        data(index, 1) = sp->getPosition().Y();
        data(index, 2) = sp->getPosition().Z();

        P(index, 0) = sp->getPosition().X() - average(0);
        P(index, 1) = sp->getPosition().Y() - average(1);
        P(index, 2) = sp->getPosition().Z() - average(2);
        index++;
      }

      Eigen::Matrix<double, 3, 3> product = P.transpose() * P;

      Eigen::EigenSolver<Eigen::Matrix<double, 3, 3>> eigencollection(product);
      Eigen::Matrix<double, 3, 1> eigenvalues = eigencollection.eigenvalues().real();
      Eigen::Matrix<std::complex<double>, 3, 3> eigenvectors = eigencollection.eigenvectors();
      Eigen::Matrix<double, 3, 1>::Index maxRow, maxCol;
      eigenvalues.maxCoeff(&maxRow, &maxCol);

      Eigen::Matrix<double, 3, 1> e = eigenvectors.col(maxRow).real();

      // Setting starting point to the data point with larges radius
      Eigen::Matrix<double, 3, 1> start = data.row(nHits - 1).transpose();
      Eigen::Matrix<double, 3, 1> second = data.row(nHits - 2).transpose();
      m_start = {start(0), start(1), start(2)};
      // Setting direction vector towards the inside of the detector
      m_direction = {e(0), e(1), e(2)};
      Eigen::Hyperplane<double, 3> plane(e.normalized(), start);
      Eigen::ParametrizedLine<double, 3> line(second, e.normalized());
      double factor = line.intersectionParameter(plane);
      if (factor > 0) {
        m_direction = { -e(0), -e(1), -e(2)};
      }

      resortHits();

      m_chi2 = 0;

      m_shittiest = std::pair<double, int>(0., 0);
      int shit_index = 0;
      for (const auto& sp : m_spacePoints) {
        Eigen::Matrix<double, 3, 1> origin(sp->getPosition().X(), sp->getPosition().Y(), sp->getPosition().Z());
        plane = Eigen::Hyperplane<double, 3>(e.normalized(), origin);

        Eigen::Matrix<double, 3, 1> point = line.intersectionPoint(plane);

        double delta_chi2 = (point - origin).transpose() * (point - origin);
        m_chi2 += delta_chi2;

        if (delta_chi2 > m_shittiest.first) {
          m_shittiest.first = delta_chi2 / nHits;
          m_shittiest.second = shit_index;
        }
        shit_index++;
      }

      m_chi2 *= 1. / nHits;
      B2WARNING("Chi2 result is " << m_chi2 << "...");
      return true;
    }

    bool compareRads(const SpacePoint* a, const SpacePoint* b)
    {
      double radA = a->getPosition().X() * a->getPosition().X() + a->getPosition().Y() * a->getPosition().Y();
      double radB = b->getPosition().X() * b->getPosition().X() + b->getPosition().Y() * b->getPosition().Y();
      return radA < radB;
    }

    void resortHits()
    {
      std::vector<const SpacePoint*> sortedSPs;
      for (auto& SP : m_spacePoints) {
        auto forwardIt = std::lower_bound(sortedSPs.begin(), sortedSPs.end(), SP,
        [this](const SpacePoint * lhs, const SpacePoint * rhs) -> bool {
          return this->comparePars(lhs, rhs);
        });
        sortedSPs.insert(forwardIt, SP);
      }
      m_spacePoints = sortedSPs;
    }

    bool comparePars(const SpacePoint* a, const SpacePoint* b)
    {
      Eigen::Matrix<double, 3, 1> posA(a->getPosition().X(), a->getPosition().Y(), a->getPosition().Z());
      Eigen::Matrix<double, 3, 1> posB(b->getPosition().X(), b->getPosition().Y(), b->getPosition().Z());

      Eigen::Matrix<double, 3, 1> direction(m_direction[0], m_direction[1], m_direction[2]);
      Eigen::Matrix<double, 3, 1> origin(m_start[0], m_start[1], m_start[2]);

      Eigen::ParametrizedLine<double, 3> line(origin, direction.normalized());
      Eigen::Hyperplane<double, 3> planeA(direction.normalized(), posA);
      Eigen::Hyperplane<double, 3> planeB(direction.normalized(), posB);

      double parA = line.intersectionParameter(planeA);
      double parB = line.intersectionParameter(planeB);
      return parA < parB;
    }


    std::vector<const SpacePoint*> m_spacePoints;

    double m_chi2 = 10;
    std::pair<double, int> m_shittiest = std::pair<double, int>(0., 0);

    std::vector<double> m_start {0., 0., 0.};
    std::vector<double> m_direction {0., 0., 0.};
  };
}


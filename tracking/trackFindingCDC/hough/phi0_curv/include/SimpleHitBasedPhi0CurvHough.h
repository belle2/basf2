/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/phi0_curv/HitInPhi0CurvBox.h>
#include <tracking/trackFindingCDC/hough/phi0_curv/Phi0CurvHough.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class HitPtr,
             size_t phi0Divisions = 2, // Division at each level
             size_t curvDivisions = 2> // Division at each level
    class SimpleHitBasedPhi0CurvHough : public Phi0CurvHough<HitPtr, phi0Divisions, curvDivisions> {

    private:
      /// Type of the base class.
      using Super = Phi0CurvHough<HitPtr, phi0Divisions, curvDivisions>;

    public:
      /// Type of the node in the hough tree.
      using Node = typename Super::Node;

    public:
      /// Constructor using the given maximal level.
      SimpleHitBasedPhi0CurvHough(size_t maxLevel,
                                  double minCurv = -0.018,
                                  double maxCurv = 0.13,
                                  double curlCurv = 0.018) :
        Super(maxLevel, minCurv, maxCurv),
        m_curlCurv(curlCurv),
        m_hitInPhi0CurvBox(curlCurv)
      {;}

    public:
      /// Find disjoint leaves heavier than minWeight
      std::vector<std::pair<Phi0CurvBox,  std::vector<HitPtr> > >
      find(const Weight& minWeight, const double& maxCurv = NAN)
      {
        auto skipHighCurvatureAndLowWeightNode = [minWeight, maxCurv](const Node * node) {
          return not(node->getWeight() >= minWeight and not(node->getLowerCurv() > maxCurv));
        };
        return this->getTree()->findLeavesDisjoint(m_hitInPhi0CurvBox,
                                                   this->getMaxLevel(),
                                                   skipHighCurvatureAndLowWeightNode);
      }

      /// Find the best trajectory and repeat the process until no bin heavier than minWeight can be found
      std::vector<std::pair<Phi0CurvBox,  std::vector<HitPtr> > >
      findBest(const Weight& minWeight, const double& maxCurv = NAN)
      {
        auto skipHighCurvatureAndLowWeightNode = [minWeight, maxCurv](const Node * node) {
          return not(node->getWeight() >= minWeight and not(node->getLowerCurv() > maxCurv));
        };
        return this->getTree()->findHeaviestLeafRepeated(m_hitInPhi0CurvBox,
                                                         this->getMaxLevel(),
                                                         skipHighCurvatureAndLowWeightNode);
      }

    private:
      /// Curvature below which a trajectory is considered non curling
      double m_curlCurv = 0.018;

      /// Predicate checking if hit is in a given phi0 curvature box
      HitInPhi0CurvBox m_hitInPhi0CurvBox;
    };
  }
}

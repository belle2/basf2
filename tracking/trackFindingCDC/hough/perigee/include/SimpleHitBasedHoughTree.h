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

#include <tracking/trackFindingCDC/hough/perigee/StereoHitContained.h>
#include <tracking/trackFindingCDC/hough/BoxDivisionHoughTree.h>
#include <tracking/trackFindingCDC/hough/perigee/CurvRep.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class HitPtr,
             class InBox,
             size_t ... divisions>
    class SimpleHitBasedHoughTree :
      public BoxDivisionHoughTree<HitPtr, typename InBox::HoughBox, divisions...> {

    private:
      /// Type of the base class.
      using Super = BoxDivisionHoughTree<HitPtr, typename InBox::HoughBox, divisions...>;

    public:
      /// Type of the node in the hough tree.
      using Node = typename Super::Node;

      /// Type of the hough box
      using HoughBox = typename InBox::HoughBox;

    public:
      /// Constructor using the given maximal level.
      SimpleHitBasedHoughTree(size_t maxLevel, float curlCurv = NAN) :
        Super(maxLevel),
        m_curlCurv(curlCurv),
        m_stereoHitContainedInBox(curlCurv)
      {;}

    public:
      /// Find disjoint leaves heavier than minWeight
      std::vector<std::pair<HoughBox,  std::vector<HitPtr> > >
      find(const Weight& minWeight, const double& maxCurv = NAN)
      {
        auto skipHighCurvatureAndLowWeightNode = [minWeight, maxCurv](const Node * node) {
          const HoughBox& houghBox = *node;
          return not(node->getWeight() >= minWeight and not(getLowerCurv(houghBox) > maxCurv));
        };
        return this->getTree()->findLeavesDisjoint(m_stereoHitContainedInBox,
                                                   this->getMaxLevel(),
                                                   skipHighCurvatureAndLowWeightNode);
      }

      /// Find the best trajectory and repeat the process until no bin heavier than minWeight can be found
      std::vector<std::pair<HoughBox,  std::vector<HitPtr> > >
      findBest(const Weight& minWeight, const double& maxCurv = NAN)
      {
        auto skipHighCurvatureAndLowWeightNode = [minWeight, maxCurv](const Node * node) {
          const HoughBox& houghBox = *node;
          return not(node->getWeight() >= minWeight and not(getLowerCurv(houghBox) > maxCurv));
        };
        return this->getTree()->findHeaviestLeafRepeated(m_stereoHitContainedInBox,
                                                         this->getMaxLevel(),
                                                         skipHighCurvatureAndLowWeightNode);
      }

      /// Fill and walk the tree using invoking the leaf processor on each encountered node.
      template<class LeafProcessor>
      void findUsing(LeafProcessor& leafProcessor)
      {
        return this->getTree()->fillWalk(m_stereoHitContainedInBox,
                                         leafProcessor);
      }

    private:
      /// Curvature below which a trajectory is considered non curling
      double m_curlCurv = 0.018;

      /// Predicate checking if a hit is in the realm of the sweeped object.
      StereoHitContained<InBox> m_stereoHitContainedInBox;
    };
  }
}

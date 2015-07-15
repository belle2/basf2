/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/hough/phi0_curv/HitInPhi0CurvBox.h>
#include <tracking/trackFindingCDC/hough/WeightedFastHough.h>
#include <tracking/trackFindingCDC/hough/LinearDivision.h>
#include <tracking/trackFindingCDC/hough/DiscreteAngles.h>

#include <cmath>
#include <vector>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class Hit,
             size_t phi0Divisions = 2, // Division at each level
             size_t curvDivisions = 2> // Division at each level
    class HitPhi0CurvLegendre {

      /// Type of the box division strategy
      using Phi0CurvBoxDivision = LinearDivision<Phi0CurvBox, phi0Divisions, curvDivisions>;

      /// Type of the fast hough tree structure
      using HitPhi0CurvFastHoughTree = WeightedFastHough<Hit, Phi0CurvBox, Phi0CurvBoxDivision>;

    private:
      // Default parameters
      // TODO: Cross check this values with old implementation, room further tuning?
      // TODO: Expose these parameters to the constructor?
      size_t m_maxLevel = 13;

      size_t m_discretePhi0Overlap = 1;
      size_t m_discretePhi0Width = 2;

      size_t m_discreteCurvOverlap = 1;
      size_t m_discreteCurvWidth = 2;
      double m_maxCurv = 3.0;

      /// Dummy initialisation of the other constructs
      DiscreteAngleArray m_discretePhi0s{NAN, NAN, 1};
      DiscreteCurvatureArray m_discreteCurvs{NAN, NAN, 1};

      Phi0CurvBox m_phi0CurvHoughPlain{m_discretePhi0s.getRange(), m_discreteCurvs.getRange()};
      std::unique_ptr<HitPhi0CurvFastHoughTree> m_hitPhi0CurvFastHoughTree{nullptr};

    public:
      void initialize()
      {
        // Setup thre discrete values for phi0
        assert(m_discretePhi0Width > m_discretePhi0Overlap);
        const size_t nPhi0Bins = std::pow(phi0Divisions, m_maxLevel);
        const size_t nDiscretePhi0s = (m_discretePhi0Width - m_discretePhi0Overlap) * nPhi0Bins + m_discretePhi0Overlap + 1;
        const double phi0Overlap = 2 * PI / (nPhi0Bins * (static_cast<double>(m_discretePhi0Width) / m_discretePhi0Overlap - 1) + 1);

        B2INFO("phi0Overlap " << phi0Overlap);

        // Adjust the phi0 bounds such that overlap occures at the wrap around of the phi0 range as well
        const double phi0LowerBound = -PI - phi0Overlap;
        const double phi0UpperBound = +PI + phi0Overlap;

        m_discretePhi0s = DiscreteAngleArray(phi0LowerBound, phi0UpperBound, nDiscretePhi0s);
        std::pair<DiscreteAngle, DiscreteAngle> phi0Range(m_discretePhi0s.getRange());

        // Setup thre discrete values for the two dimensional curvature
        assert(m_discreteCurvWidth > m_discreteCurvOverlap);
        const size_t nCurvBins = std::pow(curvDivisions, m_maxLevel);
        const size_t nDiscreteCurvs = (m_discreteCurvWidth - m_discreteCurvOverlap) * nCurvBins + m_discreteCurvOverlap + 1;

        const double curvOverlap = m_maxCurv / (nCurvBins * (static_cast<double>(m_discreteCurvWidth) / m_discreteCurvOverlap - 1) + 1);
        B2INFO("curvOverlap " << curvOverlap);

        // Since the lower bound is slightly prefered we can bias to high momenta by putting them at the lower bound.
        const double curvLowerBound = -curvOverlap;
        const double curvUpperBound = m_maxCurv + curvOverlap;

        m_discreteCurvs = DiscreteCurvatureArray(curvLowerBound, curvUpperBound, nDiscreteCurvs);
        std::pair<DiscreteCurvature, DiscreteCurvature > curvRange(m_discreteCurvs.getRange());

        // Compose the hough space
        m_phi0CurvHoughPlain = Phi0CurvBox(phi0Range, curvRange);

        Phi0CurvBox::Delta phi0CurvOverlaps{m_discretePhi0Overlap, m_discreteCurvOverlap};
        Phi0CurvBoxDivision phi0CurvBoxDivision(phi0CurvOverlaps);

        m_hitPhi0CurvFastHoughTree.reset(new HitPhi0CurvFastHoughTree(m_phi0CurvHoughPlain,
                                         phi0CurvBoxDivision));

      }

      void seed(std::vector<Hit*>& hits)
      {
        if (not m_hitPhi0CurvFastHoughTree) { initialize(); }
        m_hitPhi0CurvFastHoughTree->seed(hits);
      }

      std::vector<std::pair<Phi0CurvBox,  std::vector<Hit*> > >
      find(const Weight& minWeight, const double& maxCurv = NAN)
      {
        typedef typename HitPhi0CurvFastHoughTree::Node Node;

        const bool refined = false;
        HitInPhi0CurvBox<refined> hitInPhi0CurvBox;

        if (not std::isnan(maxCurv)) {
          auto skipHighCurvatureAndLowWeightNode = [minWeight, maxCurv](const Node * node) {
            return (not(node->getWeight() >= minWeight) or
                    static_cast<float>(node->template getLowerBound<1>()) > maxCurv);
          };
          return m_hitPhi0CurvFastHoughTree->findLeavesDisjoint(hitInPhi0CurvBox, m_maxLevel, skipHighCurvatureAndLowWeightNode);

        } else {
          auto skipLowWeightNode = [minWeight](const Node * node) {
            return not(node->getWeight() >= minWeight);
          };
          return m_hitPhi0CurvFastHoughTree->findLeavesDisjoint(hitInPhi0CurvBox, m_maxLevel, skipLowWeightNode);
        }
      }

      void fell()
      { m_hitPhi0CurvFastHoughTree->fell(); }

      void raze()
      { m_hitPhi0CurvFastHoughTree->raze(); }

    private:

    };
  }
}

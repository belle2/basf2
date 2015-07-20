/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/hough/z0_zslope/HitInZ0ZSlopeBox.h>
#include <tracking/trackFindingCDC/hough/WeightedFastHoughTree.h>
#include <tracking/trackFindingCDC/hough/LinearDivision.h>

#include <cmath>
#include <vector>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class HitPointerType,
             size_t z0Divisions = 2, // Division at each level
             size_t zSlopeDivisions = 2> // Division at each level
    class HitZ0ZSlopeLegendre {

      /// Type of the box division strategy
      using Z0ZSlopeBoxDivision = LinearDivision<Z0ZSlopeBox, z0Divisions, zSlopeDivisions>;

      /// Type of the fast hough tree structure
      using HitZ0ZSlopeFastHoughTree = WeightedFastHoughTree<HitPointerType, Z0ZSlopeBox, Z0ZSlopeBoxDivision>;

    public:
      /// Constructor using the default values
      HitZ0ZSlopeLegendre()
      {;}

      /// Constructor using the given maximal level.
      HitZ0ZSlopeLegendre(size_t maxLevel) : m_maxLevel(maxLevel)
      {;}

    public:

      /// Initialise the algorithm by constructing the hough tree from the parameters
      void initialize()
      {
        /*// Setup thre discrete values for phi0
        assert(m_discreteZ0Width > m_discreteZ0Overlap);
        const size_t nPhi0Bins = std::pow(z0Divisions, m_maxLevel);
        const size_t nDiscretePhi0s = (m_discreteZ0Width - m_discreteZ0Overlap) * nPhi0Bins + m_discreteZ0Overlap + 1;
        const double phi0Overlap = 2 * PI / (nPhi0Bins * (static_cast<double>(m_discreteZ0Width) / m_discreteZ0Overlap - 1) + 1);

        B2INFO("phi0Overlap " << phi0Overlap);

        // Adjust the phi0 bounds such that overlap occures at the wrap around of the phi0 range as well
        const double phi0LowerBound = -PI - phi0Overlap;
        const double phi0UpperBound = +PI + phi0Overlap;

        m_discreteZ0s = DiscreteAngleArray(phi0LowerBound, phi0UpperBound, nDiscretePhi0s);
        std::pair<DiscreteAngle, DiscreteAngle> phi0Range(m_discreteZ0s.getRange());

        // Setup thre discrete values for the two dimensional curvature
        assert(m_discreteZSlopeWidth > m_discreteZSlopeOverlap);
        const size_t nCurvBins = std::pow(zSlopeDivisions, m_maxLevel);
        const size_t nDiscreteCurvs = (m_discreteZSlopeWidth - m_discreteZSlopeOverlap) * nCurvBins + m_discreteZSlopeOverlap + 1;

        const double curvOverlap = m_maxCurv / (nCurvBins * (static_cast<double>(m_discreteZSlopeWidth) / m_discreteZSlopeOverlap - 1) + 1);
        B2INFO("curvOverlap " << curvOverlap);

        // Since the lower bound is slightly prefered we can bias to high momenta by putting them at the lower bound.
        const double curvLowerBound = -curvOverlap;
        const double curvUpperBound = m_maxCurv + curvOverlap;

        m_discreteZSlopes = DiscreteCurvatureArray(curvLowerBound, curvUpperBound, nDiscreteCurvs);
        std::pair<DiscreteCurvature, DiscreteCurvature > curvRange(m_discreteZSlopes.getRange());

        // Compose the hough space
        m_z0ZSlopeHoughPlain = Z0ZSlopeBox(phi0Range, curvRange);

        Z0ZSlopeBox::Delta phi0CurvOverlaps{m_discreteZ0Overlap, m_discreteZSlopeOverlap};
        Z0ZSlopeBoxDivision phi0CurvBoxDivision(phi0CurvOverlaps);

        m_hitZ0ZSlopeFastHoughTree.reset(new HitZ0ZSlopeFastHoughTree(m_z0ZSlopeHoughPlain,
                                         phi0CurvBoxDivision));*/

      }

      /// Prepare the leave finding by filling the top node with given hits
      template<class HitPointerTypes>
      void seed(HitPointerTypes& hits)
      {
        if (not m_hitZ0ZSlopeFastHoughTree) { initialize(); }
        m_hitZ0ZSlopeFastHoughTree->seed(hits);
      }

      /// Find disjoint leaves heavier than minWeight = TODO: Number of items???
      std::vector<std::pair<Z0ZSlopeBox, std::vector<HitPointerType>>>
      find(const Weight& minWeight)
      {
        typedef typename HitZ0ZSlopeFastHoughTree::Node Node;
        HitInZ0ZSlopeBox hitInZ0ZSlopeBox;

        auto skipLowWeightNode = [&minWeight](const Node * node) {
          return not(node->getWeight() >= minWeight);
        };
        return m_hitZ0ZSlopeFastHoughTree->findLeavesDisjoint(hitInZ0ZSlopeBox, m_maxLevel, skipLowWeightNode);
      }

      /// Terminates the processing by striping all hit information from the tree
      void fell()
      { m_hitZ0ZSlopeFastHoughTree->fell(); }

      /// Release all memory that the tree aquired during the runs.
      void raze()
      { m_hitZ0ZSlopeFastHoughTree->raze(); }

    private:
      // Default parameters

      /// Maximal level of the tree to investigate
      size_t m_maxLevel = 6;

      /// Overlap of the leaves in z0 counted in number of discrete values.
      size_t m_discreteZ0Overlap = 1;

      /// Width of the leaves at the maximal level in z0 counted in number of discrete values.
      size_t m_discreteZ0Width = 2;

      /// Overlap of the leaves in the inverse z slope counted in number of discrete values
      size_t m_discreteZSlopeOverlap = 1;

      /// Width of the leaves at the maximal level in the inverse z slope counted in number of discrete values.
      size_t m_discreteZSlopeWidth = 2;

      /// Maximal absolute z0 value the tree should cover.
      double m_maximumAbsZ0 = std::tan(-75.0 * PI / 180.0);

      /// Maximal absolute z slope value the tree should cover.
      double m_maximumAbsZSlope = 20.0;

      // Dummy initialisation of the other constructs

      /// Space for the discrete values that mark the usable bin bound in z0
      DiscreteZ0Array m_discreteZ0s{NAN, NAN, 1};

      /// Space for the discrete values that mark the usable bin bound in inverse z slope
      DiscreteZSlopeArray m_discreteZSlopes{NAN, NAN, 1};

      /// The top level hough plain
      Z0ZSlopeBox m_z0ZSlopeHoughPlain{m_discreteZ0s.getRange(), m_discreteZSlopes.getRange()};

      /// Dynamic hough tree structure traversed in the leave search.
      std::unique_ptr<HitZ0ZSlopeFastHoughTree> m_hitZ0ZSlopeFastHoughTree{nullptr};

    };
  }
}

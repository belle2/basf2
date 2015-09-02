/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/hough/z0_tanLambda/HitInZ0TanLambdaBox.h>
#include <tracking/trackFindingCDC/hough/WeightedFastHoughTree.h>
#include <tracking/trackFindingCDC/hough/LinearDivision.h>
#include <tracking/trackFindingCDC/hough/Unmarkable.h>

#include <cmath>
#include <vector>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class HitPointerType,
             size_t z0Divisions = 2, // Division at each level
             size_t tanLambdaDivisions = 2> // Division at each level
    class HitZ0TanLambdaLegendre {

      /// Type of the box division strategy
      using Z0TanLambdaBoxDivision = LinearDivision<Z0TanLambdaBox, z0Divisions, tanLambdaDivisions>;

      /// Type of the fast hough tree structure
      using HitZ0TanLambdaFastHoughTree = WeightedFastHoughTree<HitPointerType, Z0TanLambdaBox, Z0TanLambdaBoxDivision>;

    public:
      /// Constructor using the default values
      HitZ0TanLambdaLegendre()
      {;}

      /// Constructor using the given maximal level.
      HitZ0TanLambdaLegendre(size_t maxLevel) : m_maxLevel(maxLevel)
      {;}

    public:

      /// Initialise the algorithm by constructing the hough tree from the parameters
      void initialize()
      {
        // Setup three discrete values for phi0
        assert(m_discreteZ0Width > m_discreteZ0Overlap);
        const size_t nZ0Bins = std::pow(z0Divisions, m_maxLevel);

        const size_t nDiscreteZ0s = (m_discreteZ0Width - m_discreteZ0Overlap) * nZ0Bins + m_discreteZ0Overlap + 1;

        const double z0Overlap = 2 * m_maximumAbsZ0 / (nZ0Bins * (static_cast<double>(m_discreteZ0Width) / m_discreteZ0Overlap - 1) + 1);

        // Adjust the z0 bounds such that overlap occures at the wrap around of the z0 range as well
        const double z0LowerBound = -m_maximumAbsZ0 - z0Overlap;
        const double z0UpperBound = +m_maximumAbsZ0 + z0Overlap;


        m_discreteZ0s = linspace<float>(z0LowerBound, z0UpperBound, nDiscreteZ0s);
        const std::array<DiscreteZ0, 2> z0Range(DiscreteZ0::getRange(m_discreteZ0s));

        // Setup three discrete values for the two dimensional curvature
        assert(m_discreteTanLambdaWidth > m_discreteTanLambdaOverlap);
        const size_t nTanLambdaBins = std::pow(tanLambdaDivisions, m_maxLevel);

        const size_t nDiscreteTanLambdas = (m_discreteTanLambdaWidth - m_discreteTanLambdaOverlap) * nTanLambdaBins +
                                           m_discreteTanLambdaOverlap + 1;


        const double tanLambdaOverlap = 2 * m_maximumAbsTanLambda / (nTanLambdaBins * (static_cast<double>(m_discreteTanLambdaWidth) /
                                        m_discreteTanLambdaOverlap - 1) + 1);

        const double tanLambdaLowerBound = -m_maximumAbsTanLambda - tanLambdaOverlap;
        const double tanLambdaUpperBound = +m_maximumAbsTanLambda + tanLambdaOverlap;


        m_discreteTanLambdas = linspace<float>(tanLambdaLowerBound, tanLambdaUpperBound, nDiscreteTanLambdas);
        const std::array<DiscreteTanLambda, 2> tanLambdaRange(DiscreteTanLambda::getRange(m_discreteTanLambdas));

        // Compose the hough space
        m_z0TanLambdaHoughPlain = Z0TanLambdaBox(z0Range, tanLambdaRange);

        const Z0TanLambdaBox::Delta z0TanLambdaOverlaps{m_discreteZ0Overlap, m_discreteTanLambdaOverlap};
        const Z0TanLambdaBoxDivision z0TanLambdaBoxDivision(z0TanLambdaOverlaps);

        m_hitZ0TanLambdaFastHoughTree.reset(new HitZ0TanLambdaFastHoughTree(m_z0TanLambdaHoughPlain,
                                            z0TanLambdaBoxDivision));
      }

      /// Prepare the leave finding by filling the top node with given hits
      template<class HitPointerTypes>
      void seed(HitPointerTypes& hits)
      {
        if (not m_hitZ0TanLambdaFastHoughTree) { initialize(); }
        m_hitZ0TanLambdaFastHoughTree->seed(hits);
      }

      /// Find only the leave with the highest weight = number of items
      std::vector<std::pair<Z0TanLambdaBox, std::vector<HitPointerType>>>
      findHighest(const Weight& minWeight)
      {
        std::vector<std::pair<Z0TanLambdaBox, std::vector<HitPointerType>>> found;
        HitInZ0TanLambdaBox hitInZ0TanLambdaBox;
        auto skipLowWeightNode = [minWeight](const typename HitZ0TanLambdaFastHoughTree::Node * node) {
          return not(node->getWeight() >= minWeight);
        };
        typename HitZ0TanLambdaFastHoughTree::Node* node = m_hitZ0TanLambdaFastHoughTree->findHeaviestLeaf(hitInZ0TanLambdaBox, m_maxLevel,
                                                           skipLowWeightNode);

        if (node) {
          found.emplace_back(*node, std::vector<HitPointerType>(node->begin(), node->end()));
        }

        return found;
      }


      /// Terminates the processing by striping all hit information from the tree
      void fell()
      { m_hitZ0TanLambdaFastHoughTree->fell(); }

      /// Release all memory that the tree aquired during the runs.
      void raze()
      { m_hitZ0TanLambdaFastHoughTree->raze(); }

    private:
      // Default parameters

      /// Maximal level of the tree to investigate
      size_t m_maxLevel = 6;

      /// Overlap of the leaves in z0 counted in number of discrete values.
      size_t m_discreteZ0Overlap = 0;

      /// Width of the leaves at the maximal level in z0 counted in number of discrete values.
      size_t m_discreteZ0Width = 1;

      /// Overlap of the leaves in the tan lambda counted in number of discrete values
      size_t m_discreteTanLambdaOverlap = 0;

      /// Width of the leaves at the maximal level in the tan lambda counted in number of discrete values.
      size_t m_discreteTanLambdaWidth = 1;

      /// Maximal absolute z0 value the tree should cover.
      double m_maximumAbsZ0 = 120;

      /// Maximal absolute tan lambda value the tree should cover.
      double m_maximumAbsTanLambda = std::tan(75.0 * PI / 180.0);

      // Dummy initialisation of the other constructs

      /// Space for the discrete values that mark the usable bin bound in z0
      DiscreteZ0Array m_discreteZ0s;

      /// Space for the discrete values that mark the usable bin bound in tan lambda
      DiscreteTanLambdaArray m_discreteTanLambdas;

      /// The top level hough plain
      Z0TanLambdaBox m_z0TanLambdaHoughPlain{DiscreteZ0::getRange(m_discreteZ0s),
                                             DiscreteTanLambda::getRange(m_discreteTanLambdas)};

      /// Dynamic hough tree structure traversed in the leave search.
      std::unique_ptr<HitZ0TanLambdaFastHoughTree> m_hitZ0TanLambdaFastHoughTree{nullptr};

    };
  }
}

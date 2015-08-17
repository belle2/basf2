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
#include <tracking/trackFindingCDC/hough/phi0_curv/Phi0CurvBox.h>
#include <tracking/trackFindingCDC/hough/perigee/DiscretePhi0.h>
#include <tracking/trackFindingCDC/hough/perigee/DiscreteCurv.h>

#include <tracking/trackFindingCDC/hough/WeightedFastHoughTree.h>
#include <tracking/trackFindingCDC/hough/LinearDivision.h>
#include <tracking/trackFindingCDC/hough/DiscreteAngles.h>

#include <cmath>
#include <vector>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class ItemPtr,
             size_t phi0Divisions = 2, // Division at each level
             size_t curvDivisions = 2> // Division at each level
    class Phi0CurvHough {

    public:
      /// Type of the box division strategy
      using Phi0CurvBoxDivision = LinearDivision<Phi0CurvBox, phi0Divisions, curvDivisions>;

      /// Type of the fast hough tree structure
      using Phi0CurvFastHoughTree = WeightedFastHoughTree<ItemPtr, Phi0CurvBox, Phi0CurvBoxDivision>;

      /// Type of the nodes used in the tree for the search.
      using Node = typename  Phi0CurvFastHoughTree::Node;

    public:
      /// Constructor using the default values
      Phi0CurvHough() {;}

      /// Constructor using the given maximal level.
      Phi0CurvHough(size_t maxLevel,
                    double minCurv = -0.018,
                    double maxCurv = 0.13) :
        m_maxLevel(maxLevel),
        m_minCurv(minCurv),
        m_maxCurv(maxCurv)
      {;}

    public:
      /// Initialise the algorithm by constructing the hough tree from the parameters
      virtual void initialize()
      {
        // Setup the discrete values for phi0
        const size_t nPhi0Bins = std::pow(phi0Divisions, m_maxLevel);
        m_discretePhi0s = DiscretePhi0Array::forBinsWithOverlaps(nPhi0Bins,
                                                                 m_discretePhi0Width,
                                                                 m_discretePhi0Overlap);

        // Setup the discrete values for the two dimensional curvature
        assert(m_discreteCurvWidth > m_discreteCurvOverlap);
        const size_t nCurvBins = std::pow(curvDivisions, m_maxLevel);
        if (m_minCurv == 0) {
          m_discreteCurvs =
            DiscreteCurvArray::forPositiveCurvBinsWithOverlap(m_maxCurv,
                                                              nCurvBins,
                                                              m_discreteCurvWidth,
                                                              m_discreteCurvOverlap);
        } else {
          m_discreteCurvs =
            DiscreteCurvArray::forCurvBinsWithOverlap(m_minCurv,
                                                      m_maxCurv,
                                                      nCurvBins,
                                                      m_discreteCurvWidth,
                                                      m_discreteCurvOverlap);
        }

        // Compose the hough space
        Phi0CurvBox phi0CurvHoughPlain(m_discretePhi0s.getRange(), m_discreteCurvs.getRange());


        Phi0CurvBox::Delta phi0CurvOverlaps{m_discretePhi0Overlap, m_discreteCurvOverlap};
        Phi0CurvBoxDivision phi0CurvBoxDivision(phi0CurvOverlaps);

        m_phi0CurvFastHoughTree.reset(new Phi0CurvFastHoughTree(phi0CurvHoughPlain,
                                                                phi0CurvBoxDivision));


        B2INFO("Initialize hough plan");
        B2INFO("First curvature bin lower bound " << m_discreteCurvs[0].getValue());
        B2INFO("First curvature bin upper bound " << m_discreteCurvs[m_discreteCurvWidth].getValue());
        B2INFO("Curvature bin width " << m_discreteCurvs[m_discreteCurvWidth].getValue() - m_discreteCurvs[0].getValue());
        B2INFO("Curvature bin overlap " << m_discreteCurvs[m_discreteCurvOverlap].getValue() - m_discreteCurvs[0].getValue());

        double phi0Width = m_discretePhi0s[m_discretePhi0Width].getAngleVec().phi() - m_discretePhi0s[0].getAngleVec().phi();
        // need modulo 2 * Pi because of the wrap around of the lowest bin.
        phi0Width = std::remainder(phi0Width, 2 * PI);

        double phi0Overlap = m_discretePhi0s[m_discretePhi0Overlap].getAngleVec().phi() - m_discretePhi0s[0].getAngleVec().phi();
        // need modulo 2 * Pi because of the wrap around of the lowest bin.
        phi0Overlap = std::remainder(phi0Overlap, 2 * PI);

        B2INFO("Phi0 bin width " << phi0Width);
        B2INFO("Phi0 bin overlap " << phi0Overlap);
      }

      /// Prepare the leave finding by filling the top node with given hits
      template<class ItemPtrs>
      void seed(ItemPtrs& items)
      {
        if (not m_phi0CurvFastHoughTree) { initialize(); }
        m_phi0CurvFastHoughTree->seed(items);
      }

      /// Terminates the processing by striping all hit information from the tree
      virtual void fell()
      { m_phi0CurvFastHoughTree->fell(); }

      /// Release all memory that the tree aquired during the runs.
      virtual void raze()
      { m_phi0CurvFastHoughTree->raze(); }

    public:
      /// Getter for the tree used in the search in the hough plane.
      Phi0CurvFastHoughTree* getTree() const
      { return m_phi0CurvFastHoughTree.get(); }

      /// Getter for the currently set maximal level
      size_t getMaxLevel() const
      { return m_maxLevel; }

    private:
      // Default parameters
      // TODO: Cross check this values with old implementation, room further tuning?
      // TODO: Expose these parameters to the constructor?

      /// Maximal level of the tree to investigate
      size_t m_maxLevel = 13;

      /// Overlap of the leaves in phi0 counted in number of discrete values.
      size_t m_discretePhi0Overlap = 1;

      /// Width of the leaves at the maximal level in phi0 counted in number of discrete values.
      size_t m_discretePhi0Width = 3;

      /// Overlap of the leaves in the curvature counted in number of discrete values
      size_t m_discreteCurvOverlap = 2;

      /// Width of the leaves at the maximal level in teh curvature counted in number of discrete values.
      size_t m_discreteCurvWidth = 5;

      /// Minimal curvature value the tree should cover. Default is (minus) the maximal curvature to leave the CDC.
      double m_minCurv = -0.018;

      /// Maximal curvature value the tree should cover. Default is the maximal curvature to reach the CDC.
      double m_maxCurv = 0.13;

      /// Curvature below which a trajectory is considered non curling
      double m_curlCurv = 0.018;

      // Dummy initialisation of the other constructs
      /// Space for the discrete values that mark the usable bin bound in phi0
      DiscretePhi0Array m_discretePhi0s{NAN, NAN, 1};

      /// Space for the discrete values that mark the usable bin bound in the curvature
      DiscreteCurvArray m_discreteCurvs{NAN, NAN, 1};

      /// Dynamic hough tree structure traversed in the leave search.
      std::unique_ptr<Phi0CurvFastHoughTree> m_phi0CurvFastHoughTree{nullptr};

    };
  }
}

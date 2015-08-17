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
#include <tracking/trackFindingCDC/hough/BoxDivisionHoughTree.h>

#include <cmath>
#include <vector>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class ItemPtr,
             size_t phi0Divisions = 2, // Division at each level
             size_t curvDivisions = 2> // Division at each level
    class Phi0CurvHough :
      public BoxDivisionHoughTree<ItemPtr, Phi0CurvBox, phi0Divisions, curvDivisions> {

    public:
      /// Type of the base class
      using Super = BoxDivisionHoughTree<ItemPtr, Phi0CurvBox, phi0Divisions, curvDivisions>;

      /// Constructor using the default values
      Phi0CurvHough() {;}

      /// Constructor using the given maximal level.
      Phi0CurvHough(size_t maxLevel,
                    double minCurv = -0.018,
                    double maxCurv = 0.13) :
        Super(maxLevel),
        m_minCurv(minCurv),
        m_maxCurv(maxCurv)
      {;}

    public:
      /// Initialise the algorithm by constructing the hough tree from the parameters
      virtual void initialize()
      {
        size_t maxLevel = this->getMaxLevel();
        // Setup the discrete values for phi0
        const size_t nPhi0Bins = std::pow(phi0Divisions, maxLevel);
        Phi0BinsSpec phi0BinsSpec(nPhi0Bins,
                                  m_discretePhi0Overlap,
                                  m_discretePhi0Width);

        this->template assignArray<0>(phi0BinsSpec.constructArray(), m_discretePhi0Overlap);

        // Setup the discrete values for the two dimensional curvature
        const size_t nCurvBins = std::pow(curvDivisions, maxLevel);
        CurvBinsSpec curvBinsSpec(m_minCurv,
                                  m_maxCurv,
                                  nCurvBins,
                                  m_discreteCurvOverlap,
                                  m_discreteCurvWidth);

        this->template assignArray<1>(curvBinsSpec.constructArray(), m_discreteCurvOverlap);

        const DiscretePhi0::Array& discretePhi0s = this->template getArray<0>();
        const DiscreteCurv::Array& discreteCurvs = this->template getArray<1>();

        B2INFO("Initialize hough plan");
        B2INFO("First curvature bin lower bound " << discreteCurvs[0]);
        B2INFO("First curvature bin upper bound " << discreteCurvs[m_discreteCurvWidth]);
        B2INFO("Curvature bin width " << discreteCurvs[m_discreteCurvWidth] - discreteCurvs[0]);
        B2INFO("Curvature bin overlap " << discreteCurvs[m_discreteCurvOverlap] - discreteCurvs[0]);

        double phi0Width = discretePhi0s[m_discretePhi0Width].phi() - discretePhi0s[0].phi();
        // need modulo 2 * Pi because of the wrap around of the lowest bin.
        phi0Width = std::remainder(phi0Width, 2 * PI);

        double phi0Overlap = discretePhi0s[m_discretePhi0Overlap].phi() - discretePhi0s[0].phi();
        // need modulo 2 * Pi because of the wrap around of the lowest bin.
        phi0Overlap = std::remainder(phi0Overlap, 2 * PI);

        B2INFO("Phi0 bin width " << phi0Width);
        B2INFO("Phi0 bin overlap " << phi0Overlap);

        Super::initialize();
      }

    private:
      // Default parameters
      // TODO: Cross check this values with old implementation, room further tuning?
      // TODO: Expose these parameters to the constructor?

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
    };
  }
}

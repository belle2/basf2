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
#include <tracking/trackFindingCDC/hough/phi0_curv/HitPhi0CurvLegendreLeafProcessor.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<size_t phi0Divisions = 2, // Division at each level
             size_t curvDivisions = 2> // Division at each level
    class HitPhi0CurvLegendre : public Phi0CurvHough<CDCRLTaggedWireHit, phi0Divisions, curvDivisions> {

    private:
      /// Type of the base class.
      using Super = Phi0CurvHough<CDCRLTaggedWireHit, phi0Divisions, curvDivisions>;

      /// Type of the item in the hough tree
      using ItemPtr = CDCRLTaggedWireHit;

    public:
      /// Type of the node in the hough tree.
      using Node = typename Super::Node;

    public:
      /// Constructor using the given maximal level.
      HitPhi0CurvLegendre(size_t maxLevel,
                          double minCurv = -0.018,
                          double maxCurv = 0.13,
                          double curlCurv = 0.018) :
        Super(maxLevel, minCurv, maxCurv),
        m_curlCurv(curlCurv),
        m_hitInPhi0CurvBox(curlCurv),
        m_leafProcessor(maxLevel)
      {;}

    public:
      /// Initialise the algorithm by constructing the hough tree from the parameters
      virtual void initialize() override
      {
        Super::initialize();
      }

      /** Specialised finding appliance with dedicated leaf postprocessing of leafs
       *  simultaniuos with the walk through the hough tree.*/
      void findWithCoprocessing(const Weight& minWeight, const double& maxCurv = NAN)
      {
        m_leafProcessor.setMinWeight(minWeight);
        m_leafProcessor.setMaxCurv(maxCurv);
        this->getTree()->fillWalk(m_hitInPhi0CurvBox, m_leafProcessor);
      }

      /// Preliminary structure to store a set of hits with trajectory information
      using Candidate = typename HitPhi0CurvLegendreLeafProcessor<Node>::Candidate;

      /** Getter for the track candidates that have been constructed so far
       *  TODO: Make Candidate a CDCTrack?
       *  TODO: Allow to candidates to be moved out of the leaf processor.
       */
      const std::vector<Candidate> getCandidates()
      { return m_leafProcessor.getCandidates(); }

      virtual void fell() override
      { Super::fell(); m_leafProcessor.clear(); }

    private:
      /// Curvature below which a trajectory is considered non curling
      double m_curlCurv = 0.018;

      /// Predicate checking if hit is in a given phi0 curvature box
      HitInPhi0CurvBox m_hitInPhi0CurvBox;

      /** Processor that is walking through the hough tree capturing hits from leafs
       *  It also applies dedicated postprocessing to the found hit sets including
       *  * Fitting
       *  * Sorting of hits
       *  * Merging of hit sets
       *  * Deleting and/ or migrating of hits between sets
       */
      HitPhi0CurvLegendreLeafProcessor<Node> m_leafProcessor;

    };
  }
}

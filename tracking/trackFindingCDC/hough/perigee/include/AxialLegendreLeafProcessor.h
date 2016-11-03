/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/perigee/CurvRep.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Predicate class that is feed the nodes in a WeightedHoughTree walk
     *  It decides if a node should be further expanded and
     *  extracts items from nodes that are considered as leafs to build tracks.
     *
     *  It accumulates the tracks in a member vector from where they can be taken
     *  after the walk over the tree has been completed.
     */
    template<class ANode>
    class AxialLegendreLeafProcessor {

    public:
      /// Preliminary structure to save found hits and trajectory information
      using Candidate = std::pair<CDCTrajectory2D, std::vector<CDCRLWireHit> >;

    public:
      /// Initialize a new processor with the maximum level.
      explicit AxialLegendreLeafProcessor(const size_t maxLevel)
        : m_maxLevel(maxLevel)
      {}

      /**
       *  Entry point for the WeightedHoughTree walk to ask
       *  if a node is a leaf that should not be further divided into sub nodes.
       */
      bool operator()(ANode* node)
      {
        if (skip(node)) {
          return true;
        } else if (isLeaf(node)) {
          processLeaf(node);
          return true;
        } else {
          return false;
        }
      }

      /// Decide if the node should be expanded further or marks an end point of the depth search
      bool skip(const ANode* node)
      {
        bool tooLowWeight = not(node->getWeight() >= m_minWeight);
        bool tooHighCurvature = static_cast<float>(node->template getLowerBound<DiscreteCurv>()) > m_maxCurv;
        return tooLowWeight or tooHighCurvature;
      }

      /// Decide when a leaf node is reached that should be processed further
      bool isLeaf(const ANode* node)
      {
        return node->getLevel() >= m_maxLevel;
      }

      /**
       *  A valuable leaf has been found in the hough tree walk extract its content
       *  It may pull more hits from the whole tree.
       */
      void processLeaf(ANode* node);

    public:
      /// Getter for the candidates
      const std::vector<Candidate>& getCandidates() const
      { return m_candidates; }

      /// Clear the found candidates
      void clear()
      { m_candidates.clear(); }

    public:
      /// Getter for the maximal level of of splitting in the hough tree
      std::size_t getMaxLevel() const
      { return m_maxLevel; }

      /// Setter for the maximal level of of splitting in the hough tree
      void setMaxLevel(std::size_t maxLevel)
      { m_maxLevel = maxLevel; }

      /// Getter for the minimal weight what is need the follow the children of a node.
      const Weight& getMinWeight() const
      { return m_minWeight; }

      /// Setter for the minimal weight what is need the follow the children of a node.
      void setMinWeight(const Weight& minWeight)
      { m_minWeight = minWeight; }

      /// Getter for the maximal curvature to be investigated in the current walk.
      float getMaxCurv() const
      { return m_maxCurv; }

      /// Setter for the maximal curvature to be investigated in the current walk.
      void setMaxCurv(float curvature)
      { m_maxCurv = curvature; }

    private:
      /// Memory for the maximal level of the tree
      size_t m_maxLevel = 1;

      /// Memory for the minimal weight threshold for following the children of a node
      Weight m_minWeight = 0;

      /// Memory for the maximal curvature that should be searched in the current walk.
      float m_maxCurv = NAN;

    private:
      /// Memory for found trajectories.
      std::vector<Candidate> m_candidates;

    };
  }
}

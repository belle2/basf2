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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/hough/perigee/CurvRep.h>

#include <tracking/trackFindingCDC/hough/baseelements/WithSharedMark.h>

#include <vector>

namespace Belle2 {
  class ModuleParamList;
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
      explicit AxialLegendreLeafProcessor(int maxLevel)
        : m_param_maxLevel(maxLevel)
      {}

    public: // Node hook to interact with the tree walk
      /**
       *  Entry point for the WeightedHoughTree walk to ask
       *  if a node is a leaf that should not be further divided into sub nodes.
       */
      bool operator()(ANode* node)
      {
        ++m_nNodes;
        if (skip(node)) {
          ++m_nSkippedNodes;
          return true;
        } else if (isLeaf(node)) {
          ++m_nLeafs;
          processLeaf(node);
          return true;
        } else {
          return false;
        }
      }

      /// Decide if the node should be expanded further or marks an end point of the depth search
      bool skip(const ANode* node)
      {
        bool tooLowWeight = not(node->getWeight() >= m_param_minWeight);
        bool tooHighCurvature = (static_cast<float>(node->template getLowerBound<DiscreteCurv>()) > m_param_maxCurv or
                                 - static_cast<float>(node->template getUpperBound<DiscreteCurv>()) > m_param_maxCurv);
        return tooLowWeight or tooHighCurvature;
      }

      /// Decide when a leaf node is reached that should be processed further
      bool isLeaf(const ANode* node)
      {
        //if (node->getLevel() <= 6) return false;
        if (node->getLevel() >= m_param_maxLevel) return true;

        // Compare the resolution at the mean curvature to the resolution defined by the resolution function
        std::array<DiscreteCurv, 2> curvs = node->template getBounds<DiscreteCurv>();
        float lowerCurv = *(curvs[0]);
        float upperCurv = *(curvs[1]);
        float curvWidth = std::fabs(upperCurv - lowerCurv);
        double curvRes = m_curvResolution((lowerCurv + upperCurv) / 2.0);
        if (curvRes >= curvWidth) return true;
        return false;
      }

      /**
       *  A valuable leaf has been found in the hough tree walk. Extract its content!
       *  It may pull more hits from the whole tree by looking at the top node.
       */
      void processLeaf(ANode* leaf);

    public: // Helper method to execute some steps in the creation of a candidate from a leaf
      /**
       *  Look for more hits near a ftted trajectory from hits available in the give node.
       */
      std::vector<WithSharedMark<CDCRLWireHit> >
      searchRoad(const ANode& node, const CDCTrajectory2D& trajectory2D);

    public:
      /// Getter for the candidates structure still used in some tests.
      std::vector<Candidate> getCandidates() const;

      /// Getter for the tracks
      const std::vector<CDCTrack>& getTracks() const
      {
        return m_tracks;
      }

      /// Clear the found candidates
      void clear()
      {
        m_tracks.clear();
        m_axialWireHits.clear();
      }

      /// Set the pool of all axial wire hits to be used in the postprocessing
      void setAxialWireHits(std::vector<const CDCWireHit*> axialWireHits)
      {
        m_axialWireHits = std::move(axialWireHits);
      }

    public:
      /// Expose the parameters as a module parameter list
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix);

      /// Function to notify the leaf processor about changes in parameters before a new walk
      void beginWalk();

    public:
      /// Statistic: Number received node
      int m_nNodes = 0;

      /// Statistic: Number of skipped nodes
      int m_nSkippedNodes = 0;

      /// Statistic: Number processed leaf
      int m_nLeafs = 0;

    private:
      /// Memory for the maximal level of the tree
      int m_param_maxLevel = 1;

      /// Memory for the minimal weight threshold for following the children of a node
      double m_param_minWeight = 0;

      /// Memory for the maximal curvature that should be searched in the current walk.
      double m_param_maxCurv = NAN;

      /// Memory for the curvature of a curler in the CDC
      double m_param_curlCurv = 0.018;

      /// Memory for the number of repeated road searches
      int m_param_nRoadSearches = 0;

      /// Memory for the tree node level which should be the source of hits for the road searches. Defaults to the top most node.
      int m_param_roadLevel = 0;

      /// Memory for the name of the resolution function to be used. Valid values are 'none', 'const', 'basic', 'origin', 'offOrigin'
      std::string m_param_curvResolution = "const";

      /*
      /// Memory for the maximum allowed distance from track to hit
      double m_param_maxDistance = 0.2;

      /// Memory for the distance to be added to the track
      double m_param_newHitDistance = 0.15;
      */
    private:
      /// Memory for found trajectories.
      std::vector<CDCTrack> m_tracks;

      /// Memory for the pool of axial wire hits to can be used in the post processing
      std::vector<const CDCWireHit*> m_axialWireHits;

      /// Memory for the freely defined curvature resolution function
      std::function<double(double)> m_curvResolution;
    };
  }
}

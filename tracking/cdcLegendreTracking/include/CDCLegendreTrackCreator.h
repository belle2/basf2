/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/cdcLegendreTracking/CDCLegendreTrackDrawer.h>
#include <tracking/cdcLegendreTracking/CDCLegendrePatternChecker.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackFitter.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreQuadTree.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackingSortHit.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>

#include "genfit/TrackCand.h"
#include <cdc/dataobjects/CDCHit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>

#include <cstdlib>
#include <list>
#include <set>
#include <vector>

#include <cmath>
#include <iomanip>
#include <string>

using namespace std;

namespace Belle2 {
  namespace TrackFinderCDCLegendre {

    class QuadTree;
    class TrackDrawer;
    class TrackFitter;

    class TrackCreator {
    public:
      TrackCreator(std::vector<TrackHit*>& AxialHitList, std::vector<TrackHit*>& StereoHitList, std::list<TrackCandidate*>& trackList, std::list<TrackCandidate*>& trackletList, std::list<TrackCandidate*>& stereoTrackletList, bool appendHits, TrackFitter* cdcLegendreTrackFitter, TrackDrawer* cdcLegendreTrackDrawer);

      /**
       * @brief Function to create a track candidate
       * @param track construction of std::pairs, describing the track candidate by the axial hits, belonging to it and the parameter r and theta
       * @param trackHitList list of all track hits, which are used for track finding. Hits belonging to the track candidate will be deleted from it.
       */
      void createLegendreTrackCandidate(
        const std::pair<std::vector<TrackHit*>, std::pair<double, double> >& track,
        std::pair<double, double>& ref_point);

      /**
       * Create track candidate using CDCLegendreQuadTree nodes and return pointer to created candidate
       */
      TrackCandidate* createLegendreTrackCandidate(std::vector<QuadTree*> nodeList);

      /** Create tracklet using vector of hits and store it */
      TrackCandidate* createLegendreTracklet(std::vector<TrackHit*>& hits);

      /** Create stereo tracklet using vector of hits and store it */
      TrackCandidate* createLegendreStereoTracklet(std::vector<QuadTree*> nodeList);

      /**
       * @brief Perform the necessary operations after the track candidate has been constructed
       * @param track The constructed track candidate
       * @param trackHitList list of all hits, which are used for track finding. Hits belonging to the track candidate will be deleted from it.
       * This function leaves room for other operations like further quality checks or even the actual fitting of the track candidate.
       */
      void processTrack(TrackCandidate* track, std::list<TrackCandidate*>& trackList);

      /**
       * @brief Implementation of check for quality criteria after the track candidate was produced.
       */
      bool fullfillsQualityCriteria(TrackCandidate* cand);


      /** Creates GeantFit Track Candidates from CDCLegendreTrackCandidates */
      void createGFTrackCandidates(string& m_gfTrackCandsColName);

      /** Sort hits for fitting.
       * This method sorts hit indices to bring them in a correct order, which is needed for the fitting
       * @param hitIndices vector with the hit indices, this vector is charged within the function.
       * @param CDCLegendreTrackHits name of the CDCTrackHits array. In this way the sort funtion can get all necessary information about the hits.
       * @param charge estimated charge of the track, which is needed for hits from the same layer to be ordered correctly.
       */
      void sortHits(std::vector<TrackHit*>& hitIndices, int charge);

      /** Appends new unused hits from set of hits which might belong to current track */
      void appendNewHits(TrackCandidate* track);

      /** Move candidate between two collections */
      void moveCandidate(list<TrackCandidate*>&, list<TrackCandidate*>&, TrackCandidate*);

      /** Remove candidate from given list */
      void removeFromList(list<TrackCandidate*>&, TrackCandidate*);

    private:
      std::vector<TrackHit*>& m_AxialHitList; /**< Vector which hold axial hits */
      std::vector<TrackHit*>& m_StereoHitList; /**< Vector which holds stereo hits */
      std::list<TrackCandidate*>& m_trackList; /**< List of track candidates. */
      std::list<TrackCandidate*>& m_trackletList; /**< List of tracklets. */
      std::list<TrackCandidate*>& m_stereoTrackletList; /**< List of stereo tracklets. */
      bool m_appendHits; /**< Trying to append new hits to track candidate*/

      TrackFitter* m_cdcLegendreTrackFitter; /**< Class which performs track candidate fitting */
      TrackDrawer* m_cdcLegendreTrackDrawer; /**< Class which performs in-module drawing */


    };
  }
}


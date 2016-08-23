/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>
#include <algorithm>

namespace Belle2 {
  /** Executes greedy algorithm for vector of QITrackOverlap structs.
   *
   *  In contrast to Jakob's original greedy implementation, this one doesn't use a recursive
   *  algorithm, but sorts according to QI and simply goes from the top and kills all overlapping
   *  SpacePointTrackCands.
   */
  class Scrooge {
  public:
    /** Struct for holding SpacePointTrackCand information necessary for greedy.*/
    struct QITrackOverlap {
      /** Constructor with initializer list for all relevant info for greedy.
       *
       *  Scrooge takes a vector of this structures to perform the algorithm.
       */
      QITrackOverlap(float qI, unsigned short tI, std::vector<unsigned short> o, bool iA):
        qualityIndex(qI), trackIndex(tI), overlaps(o), isActive(iA) {}
      float                       qualityIndex;///<Estimate of quality, e.g. from Circle Fitter or other fast fitter.
      unsigned short              trackIndex;  ///<Index of the SpacePointTrackCand in the StoreArray.
      std::vector<unsigned short> overlaps;    ///<Vector of indices of tracks, that overlap with this candidate.
      bool                        isActive;    ///<False, if the track was killed due to overlap.
    };

    /** Make a new Scrooge for every set, you want to evaluate.*/
    Scrooge(std::vector <QITrackOverlap>& qiTrackOverlap) :
      m_qiTrackOverlap(qiTrackOverlap)
    {}

    /** Sets the isActive flag in m_qiTrackOverlap to false, for killed tracks.
     *
     *  Sorts the "tracks" according to quality, loops from the top and kills
     *  overlapping tracks of all tracks, that are still active.
     */
    void performSelection()
    {
      //sort the vector according to the QI supplied.
      std::sort(m_qiTrackOverlap.begin(), m_qiTrackOverlap.end(),
      [](QITrackOverlap const & lhs, QITrackOverlap const & rhs) -> bool {
        return lhs.qualityIndex > rhs.qualityIndex;
      });

      //kill all tracks, that have overlaps and lower QI:
      auto endOfQITrackOverlap   = m_qiTrackOverlap.cend();
      for (auto trackIter = m_qiTrackOverlap.begin(); trackIter != endOfQITrackOverlap; trackIter++) {
        if (!trackIter->isActive) continue;
        for (auto testTrackIter = trackIter; testTrackIter != endOfQITrackOverlap; testTrackIter++) {
          if (std::find(trackIter->overlaps.begin(), trackIter->overlaps.end(), testTrackIter->trackIndex) !=
              trackIter->overlaps.end()) {
            testTrackIter->isActive = false;
          }
        }
      }
    }

  private:
    std::vector <QITrackOverlap>& m_qiTrackOverlap;///<Data structure, on which algorithm is performed.
  };
}

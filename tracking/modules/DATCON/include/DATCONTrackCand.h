/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <root/TVector2.h>

namespace Belle2 {
  /**
    * The DATCONTrackCand represents the candidates of tracks found by the
    * DATCON algoritms for u-side and v-side SVD hits independently.
    * These track candidates are later combined into 3D tracks.
    * A track candidate consists of a list of hits (representing the
    * associated space points), hitList, and a TVector2 containing the coordinates
    * of intersections in the Hough Space which give either (phi, omega = 1/R) or
    * (theta, Zzero). Internal variables are a hash as sum of the entries
    * of hitList, and the number of hits in hitList, hitSize.
    */
  class DATCONTrackCand {
  public:
    /** Constructor for hough candidates */
    DATCONTrackCand(std::vector<unsigned int>& list, TVector2 coord): hitList(list), coordinate(coord)
    {
      hash = 0;
      hitSize = 0;
      for (unsigned int i = 0; i < hitList.size(); ++i) {
        hash += hitList[i];
        ++hitSize;
      }
    }

    ~DATCONTrackCand() {}

    /** Get Index list */
    std::vector<unsigned int> getIdList() { return hitList; }

    /** Get Index list */
    TVector2 getCoord() { return coordinate; }

    /** Get Hash of hit list */
    unsigned int getHash() const { return hash; }

    /** Get Size of hit list */
    unsigned int getHitSize() const { return hitSize; }

  private:
    /** List of IDs of hits that belong to this DATCONTrackCand */
    std::vector<unsigned int> hitList;
    /** Coordinate of this candidate */
    TVector2 coordinate;
    /** Hash for id list */
    unsigned int hash;
    /** Size of this DATCONTrackCand */
    unsigned int hitSize;
  }; // end class definition

}; // end namespace Belle2
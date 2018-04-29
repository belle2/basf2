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

// #include <tracking/modules/DATCON/DATCONModule.h>

namespace Belle2 {
  /*
   * Hough Candidates class.
   */
  class DATCONTrackCand {
  public:
    /** Constructor for hough candidates */
//       DATCONTrackCand(std::vector<unsigned int>& _list, TVector2 _coord, bool _left = false): hitList(_list), coord(_coord), left(_left)
    DATCONTrackCand(std::vector<unsigned int>& _list, TVector2 _coord): hitList(_list), coord(_coord)
    {
      hash = 0;
      hitSize = 0;
      for (unsigned int i = 0; i < hitList.size(); ++i) {
//           hash += hitList[i] + i * 10000000;
        hash += hitList[i];
        ++hitSize;
      }
    }

    ~DATCONTrackCand() {}

    /** Get Index list */
    std::vector<unsigned int> getIdList() { return hitList; }

    /** Get Index list */
    TVector2 getCoord() { return coord; }

    /** Get Hash of hit list */
    unsigned int getHash() const { return hash; }

    /** Get Size of hit list */
    unsigned int getHitSize() const { return hitSize; }

//       /** Get track orientation */
//       bool getTrackOrientation() { return left; }

  private:
    /** ID list of points */
    std::vector<unsigned int> hitList;
    /** Coordinate of rectangle for this candidate */
    TVector2 coord;
    /** Hash for id list */
    unsigned int hash;
    unsigned int hitSize;
//       /** Left handed track */
//       bool left;
  }; // end class definition

}; // end namespace Belle2
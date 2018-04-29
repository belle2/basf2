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
//   namespace SVD {

  /** Pair of two TVector2 containing the lower left and upper right
   * corner of a Hough Space (sub) sector
   */
  typedef std::pair<TVector2, TVector2> coord2dPair;

  /*
  * Hough Candidates class.
  */
  class DATCONHoughCand {
  public:
    /** Constructor for hough candidates */
    //       DATCONHoughCand(std::vector<unsigned int>& _list, coord2dPair _coord, bool _left = false): hitList(_list), coord(_coord), left(_left)
    DATCONHoughCand(std::vector<unsigned int>& _list, coord2dPair _coord): hitList(_list), coord(_coord)
    {
      hash = 0;
      hitSize = 0;
      for (unsigned int i = 0; i < hitList.size(); ++i) {
        //           hash += (hitList[i] + i * 10000000);
        hash += hitList[i];
        ++hitSize;
      }
    }

    ~DATCONHoughCand() {}

    /** Get Index list */
    std::vector<unsigned int> getIdList() { return hitList; }

    /** Get Index list */
    coord2dPair getCoord() { return coord; }

    /** Get Hash of hit list */
    unsigned int getHash() const { return hash; }

    /** Get Size of hit list */
    unsigned int getHitSize() const { return hitSize; }

    //   /** Get track orientation */
    //   bool getTrackOrientation() { return left; }

    /*    inline bool operator<(const DATCONHoughCand& a, const DATCONHoughCand& b)
        {
          return (a.getHash() > b.getHash());
        } */
    inline bool operator<(const DATCONHoughCand& b)
    {
      return (this->getHash() > b.getHash());
    }

  private:
    /** ID list of points */
    std::vector<unsigned int> hitList;
    /** Coordinate of rectangle for this candidate */
    coord2dPair coord;
    /** Hash for id list */
    unsigned int hash;
    unsigned int hitSize;
    //   /** Left handed track */
    //   bool left;
  }; // end class definition
//   }; // end namespace SVD
}; // end namespace Belle2
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
    * A HoughCand is a pair of two TVector2 containing the lower left and upper right
    * corner of a Hough Space (sub) sector as well as the list of the hits (=space points)
    * associated with this HoughCand (hits of at least three different SVD layers).
    */
  typedef std::pair<TVector2, TVector2> coord2dPair;

  /*
  * Hough Candidates class.
  */
  class DATCONHoughCand {
  public:
    /** Constructor for hough candidates */
    DATCONHoughCand(std::vector<unsigned int>& list, coord2dPair coord): hitList(list), coordinatePair(coord)
    {
      hash = 0;
      hitSize = 0;
      for (unsigned int i = 0; i < hitList.size(); ++i) {
        hash += hitList[i];
        ++hitSize;
      }
    }

    ~DATCONHoughCand() {}

    /** Get Index list */
    std::vector<unsigned int> getIdList() { return hitList; }

    /** Get Index list */
    coord2dPair getCoord() { return coordinatePair; }

    /** Get Hash of hit list */
    unsigned int getHash() const { return hash; }

    /** Get Size of hit list */
    unsigned int getHitSize() const { return hitSize; }

    /** Smaller-than operator for sorting functions */
    inline bool operator<(const DATCONHoughCand& b)
    {
      return (this->getHash() > b.getHash());
    }

  private:
    /** List of IDs of hits that belong to this DATCONHoughCand */
    std::vector<unsigned int> hitList;
    /** Coordinate of rectangle for this candidate */
    coord2dPair coordinatePair;
    /** Hash for id list */
    unsigned int hash;
    /** Size of this DATCONHoughCand */
    unsigned int hitSize;
  }; // end class definition
}; // end namespace Belle2
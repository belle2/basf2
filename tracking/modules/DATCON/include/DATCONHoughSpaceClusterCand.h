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

#include <framework/core/Module.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/simulation/SVDSignal.h>
#include <svd/geometry/SensorInfo.h>
#include <svd/reconstruction/Sample.h>
#include <pxd/geometry/SensorInfo.h>
#include <framework/dataobjects/RelationElement.h>
#include <boost/array.hpp>
#include <string>
#include <set>
#include <vector>
#include <fstream>

#include <root/TVector3.h>
#include <root/TVector2.h>
#include <root/TFile.h>
#include <root/TTree.h>
#include <root/TH1D.h>
#include <root/TH2D.h>
#include <root/TF1.h>

namespace Belle2 {
  /*
  * Hough Candidates class.
  */
  class DATCONHoughSpaceClusterCand {
  public:
    /** Constructor for Hough Space Cluster Candidates */
    DATCONHoughSpaceClusterCand(std::vector<unsigned int>& list, TVector2 coord): hitList(list), coordinate(coord)
    {
      hash = 0;
      hitSize = 0;
      for (unsigned int i = 0; i < hitList.size(); ++i) {
        hash += hitList[i];
        ++hitSize;
      }
    }

    ~DATCONHoughSpaceClusterCand() {}

    /** Get Index list */
    std::vector<unsigned int> getIdList() { return hitList; }

    /** Get Index list */
    TVector2 getCoord() { return coordinate; }

    /** Get Hash of hit list */
    unsigned int getHash() const { return hash; }

    /** Get Size of hit list */
    unsigned int getHitSize() const { return hitSize; }

  private:
    /** List of IDs of hits that belong to this DATCONHoughSpaceClusterCand */
    std::vector<unsigned int> hitList;
    /** Coordinate of this candidate */
    TVector2 coordinate;
    /** Hash for id list */
    unsigned int hash;
    /** Size of this DATCONHoughSpaceClusterCand */
    unsigned int hitSize;

  }; // end class definition
}; // end namespace Belle2
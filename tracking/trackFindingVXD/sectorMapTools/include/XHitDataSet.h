/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>


namespace Belle2 {

  /** Trivial struct used by the SecMapTrainerXYModules to store/cluster hitCombinations found. */
  template <class SecIDType, class HitType>
  struct XHitDataSet {

//  /** name of SectorMap. */
//  std::string secMapName;

    /** SectorIDs. */
    std::vector<SecIDType> sectorIDs;

    /** hits (must have an equal or greater number of entries as for the sectorIDs. */
    std::vector<const HitType*> hits;

//  /** TrackID. */
//  unsigned trackID;
  };
}


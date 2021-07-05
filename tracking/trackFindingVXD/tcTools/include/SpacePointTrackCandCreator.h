/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

namespace Belle2 {

  /** small class to take simple vectors of SpacePoints and convert them to real SpacePointTrackCands */
  template<class SPTCContainerType>
  struct SpacePointTrackCandCreator {

    /** takes simple vectors of SpacePoints and convert them to real SpacePointTrackCand.
     *  Returns the created Space Point. */
    SpacePointTrackCand* createSPTC(SPTCContainerType& tcContainer, std::vector<const SpacePoint*>& spacePoints, short family = -1)
    {
      SpacePointTrackCand* newSPTC = tcContainer.appendNew(spacePoints);
      newSPTC->setFamily(family);

      return newSPTC;
    }
  };

} //Belle2 namespace

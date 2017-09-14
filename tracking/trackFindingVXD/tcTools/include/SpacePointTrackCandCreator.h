/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <TVector3.h>
#include <TVectorD.h>
#include <TMatrixDSym.h>

namespace Belle2 {

  /** small class to take simple vectors of SpacePoints and convert them to real SpacePointTrackCands */
  template<class SPTCContainerType>
  struct SpacePointTrackCandCreator {

    /** takes simple vectors of SpacePoints and convert them to real SpacePointTrackCand.
     * returns number of TCs successfully created. */
    SpacePointTrackCand* createSPTCs(SPTCContainerType& tcContainer, std::vector<const SpacePoint*>& spacePoints, short family = -1)
    {
      SpacePointTrackCand* newSPTC = tcContainer.appendNew(spacePoints);
      newSPTC->setFamily(family);

      return newSPTC;
    }
  };

} //Belle2 namespace

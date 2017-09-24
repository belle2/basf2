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
    unsigned int createSPTCs(SPTCContainerType& tcContainer, std::vector<std::vector<const SpacePoint*> >& allPaths)
    {
      unsigned int nTCsCreated = 0;

      for (std::vector<const SpacePoint*>& aPath : allPaths) {
        auto* newSPTC = tcContainer.appendNew(aPath);
        nTCsCreated++;
      }

      return nTCsCreated;
    }
  };

} //Belle2 namespace

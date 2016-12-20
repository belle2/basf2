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

#include <tracking/vxdCaTracking/SharedFunctions.h> // e.g. PositionInfo
#include <tracking/vxdCaTracking/TrackletFilters.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <TVector3.h>
#include <TVectorD.h>
#include <TMatrixDSym.h>

namespace Belle2 {

  /**
   * WARNING
   *
   * using old TrackLetFilters for seed-calculation as a workaround for the time being.
   * Their input-containers expect local errors for sigmaU and sigmaV too, which are actually never used in the TrackletFilters (except for Debug-output) and will therefore set to 0.
   */

  /** small class to take simple vectors of SpacePoints and convert them to real SpacePointTrackCand including realistic seed */
  template<class SPTCContainerType>
  struct SpacePointTrackCandCreator {

    /** takes simple vectors of SpacePoints and convert them to real SpacePointTrackCand and sets relation between SPs and SPTCs,
     * returns number of TCs successfully created. */
    unsigned int createSPTCs(SPTCContainerType& tcContainer, std::vector<std::vector<const SpacePoint*> > allPaths)
    {
      unsigned int nTCsCreated = 0;

      for (std::vector<const SpacePoint*>& aPath : allPaths) {
        nTCsCreated++;

        auto* newSPTC = tcContainer.appendNew(aPath);

        // Set relations between Nodes and Space Point Track Candidates
        for (const SpacePoint* aNode : aPath) { // is a const SpacePoint* here
          if (aNode->getType() == VXD::SensorInfoBase::VXD) continue; /**< Don't create a relation for the VirtualIP. */
          aNode->addRelationTo(newSPTC, 1.);
          newSPTC->addRelationTo(aNode, 1.);
        }

      }

      return nTCsCreated;
    }
  };

} //Belle2 namespace

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

namespace Belle2 {
  class CKFCDCToVXDResultObject {
  public:
    CKFCDCToVXDResultObject(const RecoTrack* seed)
    {
      m_addedSpacePoints.reserve(5);
    }

    CKFCDCToVXDResultObject append(const SpacePoint* hit) const
    {
      CKFCDCToVXDResultObject copiedObject(*this);

      copiedObject.m_addedSpacePoints.push_back(hit);
      copiedObject.m_lastLayer--;
      return copiedObject;
    }

    unsigned int getLastLayer() const
    {
      return m_lastLayer;
    }

    RecoTrack* getSeedRecoTrack() const
    {
      return m_seedRecoTrack;
    }

  private:
    RecoTrack* m_seedRecoTrack;
    std::vector<const SpacePoint*> m_addedSpacePoints;
    unsigned int m_lastLayer = 7;
  };
}
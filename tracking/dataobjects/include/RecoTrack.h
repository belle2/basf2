/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Nils Braun                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/dataobjects/SorterBaseCDCHit.h>
#include <tracking/dataobjects/SorterBaseVXDHit.h>

#include <pxd/dataobjects/PXDCluster.h>

#include <svd/dataobjects/SVDCluster.h>

#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>

#include <framework/dataobjects/Helix.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <framework/datastore/RelationsObject.h>

#include <genfit/Track.h>

#include <vector>
#include <string>


class FitConfiguration;

namespace Belle2 {
  // Forward declerations
  class CDCHit;

  /** This is the Reconstruction Event-Data Model Track.
   *
   *  ///FIXME clean this comment!
   *  This class collects hits, performs fits and saves the Track parameters.
   *  Note: This class is still experimental.
   *  Totally missing:
   *   - Fitting functionality
   *   - Storing of which Trackfinders have contributed hits to the RecoTrack
   *     and the exact relation with the HitInfoManager.
   *   - Sorting magic.
   */
  class RecoTrack : public RelationsObject {
  public:
    typedef RecoHitInformation::RightLeftInformation RightLeftInformation;
    typedef RecoHitInformation::TrackingDetector TrackingDetector;
    typedef RecoHitInformation::OriginTrackFinder OriginTrackFinder;

    /**
     * Empty constructor for ROOT.
     */
    RecoTrack(const std::string& storeArrayNameOfCDCHits,
              std::string& storeArrayNameOfRecoHitInformation) :
      m_helix(), m_storeArrayNameOfCDCHits(storeArrayNameOfCDCHits),
      m_storeArrayNameOfRecoHitInformation(storeArrayNameOfRecoHitInformation)
    {}

    bool addCDCHit(CDCHit* cdcHit, RightLeftInformation rightLeftInformation = RightLeftInformation::undefinedRightLeftInformation,
                   OriginTrackFinder foundByTrackFinder = OriginTrackFinder::undefinedTrackFinder,
                   double travelS = 0, const TVector3& reconstructedPosition = TVector3(0, 0, 0));


    bool hasCDCHits()
    {
      return getRelatedFrom<CDCHit>(m_storeArrayNameOfCDCHits) != nullptr;
    }

    bool hasCDCHit(CDCHit* cdcHit)
    {
      RelationVector<RecoTrack> relatedTracksToCDCHit = cdcHit->getRelationsTo<RecoTrack>(getArrayName());
      return std::find_if(relatedTracksToCDCHit.begin(), relatedTracksToCDCHit.end(), [this](const RecoTrack & recoTrack) {
        return &recoTrack == this;
      }) != relatedTracksToCDCHit.end();
    }

    RecoHitInformation* getRecoHitInformation(CDCHit* cdcHit)
    {
      RelationVector<RecoHitInformation> relatedHitInformationToRecoTrack = getRelationsTo<RecoHitInformation>
          (m_storeArrayNameOfRecoHitInformation);

      for (RecoHitInformation& recoHitInformation : relatedHitInformationToRecoTrack) {
        if (recoHitInformation.getRelated<CDCHit>(m_storeArrayNameOfCDCHits) == cdcHit) {
          return &recoHitInformation;
        }
      }

      return nullptr;
    }

    const Helix& getHelix() const
    {
      return m_helix;
    }

    void setHelix(const Helix& helix)
    {
      m_helix = helix;
    }

  private:
    Helix m_helix;
    std::string m_storeArrayNameOfCDCHits;
    std::string m_storeArrayNameOfRecoHitInformation;

    bool addCDCHit(CDCHit* cdcHit, RecoHitInformation* recoHitInformation);

    //-----------------------------------------------------------------------------------
    /** Making this class a ROOT class.*/
    ClassDef(RecoTrack, 2);
  };
}

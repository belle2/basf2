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


#include <framework/datastore/StoreArray.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>

class FitConfiguration;

namespace Belle2 {
  // Forward declarations
  class CDCHit;

  /** This is the Reconstruction Event-Data Model Track.
   *
   *  ///FIXME clean this comment!
   *  This class collects hits, performs fits and saves the Track parameters.
   *  Note: This class is still experimental.
   *  Totally missing:
   *   - Fitting functionality -> relate to genfit::Track
   */
  class RecoTrack : public RelationsObject {
  public:
    typedef RecoHitInformation::RightLeftInformation RightLeftInformation;
    typedef RecoHitInformation::TrackingDetector TrackingDetector;
    typedef RecoHitInformation::OriginTrackFinder OriginTrackFinder;

    /**
     * Empty constructor for ROOT.
     */
    RecoTrack() :
      m_helix(),
      m_storeArrayNameOfCDCHits(),
      m_storeArrayNameOfSVDHits(),
      m_storeArrayNameOfPXDHits(),
      m_storeArrayNameOfRecoHitInformation()
    {}


    RecoTrack(const Helix& helix,
              const std::string& storeArrayNameOfCDCHits = "CDCHits",
              const std::string& storeArrayNameOfSVDHits = "SVDHits",
              const std::string& storeArrayNameOfPXDHits = "PXDHits",
              const std::string& storeArrayNameOfRecoHitInformation = "RecoHitInformations") :
      m_helix(helix),
      m_storeArrayNameOfCDCHits(storeArrayNameOfCDCHits),
      m_storeArrayNameOfSVDHits(storeArrayNameOfSVDHits),
      m_storeArrayNameOfPXDHits(storeArrayNameOfPXDHits),
      m_storeArrayNameOfRecoHitInformation(storeArrayNameOfRecoHitInformation)
    {}


    RecoTrack(const TVector3& position, const TVector3& momentum, const short int charge,
              const double bZ,
              const std::string& storeArrayNameOfCDCHits = "CDCHits",
              const std::string& storeArrayNameOfSVDHits = "SVDHits",
              const std::string& storeArrayNameOfPXDHits = "PXDHits",
              const std::string& storeArrayNameOfRecoHitInformation = "RecoHitInformations") :
      m_helix(position, momentum, charge, bZ),
      m_storeArrayNameOfCDCHits(storeArrayNameOfCDCHits),
      m_storeArrayNameOfSVDHits(storeArrayNameOfSVDHits),
      m_storeArrayNameOfPXDHits(storeArrayNameOfPXDHits),
      m_storeArrayNameOfRecoHitInformation(storeArrayNameOfRecoHitInformation)
    {}


    /** Define, use of clusters or true hits for SVD.
     *
     *  You have to decide, if you want to use Clusters or true hits at compile-time.
     *  In the real experiment, we want to use clusters without overhead from checking every time,
     *  if we should use true hits instead.
     */
    typedef SVDCluster SVDHit;

    /** Define, use of clusters or true hits for PXD. */
    typedef PXDCluster PXDHit;

#ifndef __CINT__
    // Hits Adding
    bool addCDCHit(CDCHit* cdcHit, RightLeftInformation rightLeftInformation = RightLeftInformation::undefinedRightLeftInformation,
                   OriginTrackFinder foundByTrackFinder = OriginTrackFinder::undefinedTrackFinder) const
    {
      double normedOmegaOfTrack = std::abs(getHelix().getOmega());
      B2ASSERT("Can not calculate the arc length of hits for a track without omega = 0. Please set the arc length by yourself.",
               normedOmegaOfTrack > 1E-100);
      const TrackFindingCDC::CDCWire* cdcWire = TrackFindingCDC::CDCWire::getInstance(*cdcHit);
      const TrackFindingCDC::Vector2D& referenceHitPosition = cdcWire->getRefPos2D();
      double arcLengthOfHit = calculateArcLength(referenceHitPosition.x(), referenceHitPosition.y());
      return addCDCHit(cdcHit, arcLengthOfHit * normedOmegaOfTrack, rightLeftInformation, foundByTrackFinder);
    }
#endif

    bool addCDCHit(CDCHit* cdcHit, const double reconstructedArcLength,
                   RightLeftInformation rightLeftInformation = RightLeftInformation::undefinedRightLeftInformation,
                   OriginTrackFinder foundByTrackFinder = OriginTrackFinder::undefinedTrackFinder) const
    {
      return addHit(cdcHit, rightLeftInformation, foundByTrackFinder, reconstructedArcLength);
    }

    RecoHitInformation* getRecoHitInformation(CDCHit* cdcHit) const
    {
      return getRecoHitInformation(cdcHit, m_storeArrayNameOfCDCHits);
    }

    RecoHitInformation* getRecoHitInformation(SVDHit* cdcHit) const
    {
      return getRecoHitInformation(cdcHit, m_storeArrayNameOfSVDHits);
    }

    RecoHitInformation* getRecoHitInformation(PXDHit* cdcHit) const
    {
      return getRecoHitInformation(cdcHit, m_storeArrayNameOfPXDHits);
    }

    // Hits Information Questioning
    template <class HitType>
    TrackingDetector getTrackingDetector(HitType* hit) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformation(hit);
      if (recoHitInformation == nullptr)
        return TrackingDetector::invalidTrackingDetector;
      else
        return recoHitInformation->getTrackingDetector();
    }

    template <class HitType>
    RightLeftInformation getRightLeftInformation(HitType* hit) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformation(hit);
      if (recoHitInformation == nullptr)
        return RightLeftInformation::invalidRightLeftInformation;
      else
        return recoHitInformation->getRightLeftInformation();
    }

    template <class HitType>
    OriginTrackFinder getFoundByTrackFinder(HitType* hit) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformation(hit);
      if (recoHitInformation == nullptr)
        return OriginTrackFinder::invalidTrackFinder;
      else
        return recoHitInformation->getFoundByTrackFinder();
    }

    template <class HitType>
    double getReconstructedArcLength(HitType* hit) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformation(hit);
      if (recoHitInformation == nullptr)
        return std::nan("");
      else
        return recoHitInformation->getReconstructedArcLength();
    }

    // Hits Information Questioning
    template <class HitType>
    bool setRightLeftInformation(HitType* hit, RightLeftInformation rightLeftInformation) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformation(hit);
      if (recoHitInformation == nullptr)
        return false;
      else
        recoHitInformation->setRightLeftInformation(rightLeftInformation);

      return true;
    }

    template <class HitType>
    bool setFoundByTrackFinder(HitType* hit, OriginTrackFinder originTrackFinder) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformation(hit);
      if (recoHitInformation == nullptr)
        return false;
      else
        recoHitInformation->setFoundByTrackFinder(originTrackFinder);

      return true;
    }

    // Hits Added Questioning
    bool hasCDCHits() const
    {
      return getRelatedFrom<CDCHit>(m_storeArrayNameOfCDCHits) != nullptr;
    }

    bool hasSVDHits() const
    {
      return getRelatedFrom<Belle2::RecoTrack::SVDHit>(m_storeArrayNameOfSVDHits) != nullptr;
    }

    bool hasPXDHits() const
    {
      return getRelatedFrom<Belle2::RecoTrack::PXDHit>(m_storeArrayNameOfPXDHits) != nullptr;
    }

    template <class HitType>
    bool hasHit(HitType* hit) const
    {
      const RelationVector<RecoTrack>& relatedTracksToHit = hit->template getRelationsTo<RecoTrack>(getArrayName());
      return std::find_if(relatedTracksToHit.begin(), relatedTracksToHit.end(), [this](const RecoTrack & recoTrack) {
        return &recoTrack == this;
      }) != relatedTracksToHit.end();
    }

    // Hits Questioning
    unsigned int getNumberOfCDCHits() const
    {
      return getNumberOfHits<CDCHit>(m_storeArrayNameOfCDCHits);
    }

    unsigned int getNumberOfSVDHits() const
    {
      return getNumberOfHits<Belle2::RecoTrack::SVDHit>(m_storeArrayNameOfSVDHits);
    }

    unsigned int getNumberOfPXDHits() const
    {
      return getNumberOfHits<Belle2::RecoTrack::PXDHit>(m_storeArrayNameOfPXDHits);
    }

    std::vector<Belle2::CDCHit*> getCDCHitList() const
    {
      return getHitList<CDCHit>(m_storeArrayNameOfCDCHits);
    }

    std::vector<Belle2::RecoTrack::SVDHit*> getSVDHitList() const
    {
      return getHitList<Belle2::RecoTrack::SVDHit>(m_storeArrayNameOfSVDHits);
    }

    std::vector<Belle2::RecoTrack::PXDHit*> getPXDHitList() const
    {
      return getHitList<Belle2::RecoTrack::PXDHit>(m_storeArrayNameOfPXDHits);
    }

    std::vector<Belle2::CDCHit*> getSortedCDCHitList() const
    {
      return getSortedHitList<CDCHit>(m_storeArrayNameOfCDCHits);
    }

    std::vector<Belle2::RecoTrack::SVDHit*> getSortedSVDHitList() const
    {
      return getSortedHitList<Belle2::RecoTrack::SVDHit>(m_storeArrayNameOfSVDHits);
    }

    std::vector<Belle2::RecoTrack::PXDHit*> getSortedPXDHitList() const
    {
      return getSortedHitList<Belle2::RecoTrack::PXDHit>(m_storeArrayNameOfPXDHits);
    }

    // Helix Stuff
    const Helix& getHelix() const
    {
      return m_helix;
    }

    void replaceHelix(const Helix& helix)
    {
      m_helix = helix;
      mapOnHits<CDCHit>(m_storeArrayNameOfCDCHits, [](RecoHitInformation & recoHitInformation, CDCHit * const hit) -> void {

      });
    }

    TVector3 getPerigee() const
    {
      return getHelix().getPerigee();
    }

    TVector3 getMomentum() const
    {
      return getHelix().getMomentum();
    }

    double calculateArcLength(const double x, const double y) const
    {
      return getHelix().getArcLength2DAtXY(x, y);
    }

    // Hit Pattern stuff
    HitPatternCDC getHitPatternCDC(const short pseudoCharge) const
    {
      HitPatternCDC hitPatternCDC;

      mapOnHits<CDCHit>(m_storeArrayNameOfCDCHits, [&hitPatternCDC](const RecoHitInformation&, const CDCHit * const hit) -> void {
        // I need to initialize a WireID with the ID from the CDCHit to get the continuous layer ID.
        WireID wireID(hit->getID());
        // Then I set the corresponding layer in the hit pattern.
        hitPatternCDC.setLayer(wireID.getICLayer());
      }, [&pseudoCharge](const RecoHitInformation & hitInformation, const CDCHit * const) -> bool {
        // Little trick: if we want the first half, we want the s to be 0 <= s <= pi,
        // if we want the second half, we want the s to be -pi <= s <= 0. Because -pi <= s <= pi is assured
        // by the RecoHitInformation, we only have to test of s > 0 or s < 0. For speed we test if s > 0 or -s > 0.
        return pseudoCharge * hitInformation.getReconstructedArcLength() > 0;
      });
      return hitPatternCDC;
    }

    HitPatternVXD getHitPatternVXD(const short pseudoCharge) const
    {
      HitPatternVXD hitPatternVXD;
      /* TODO */
      return hitPatternVXD;
    }

  private:
    Helix m_helix;
    std::string m_storeArrayNameOfCDCHits;
    std::string m_storeArrayNameOfSVDHits;
    std::string m_storeArrayNameOfPXDHits;
    std::string m_storeArrayNameOfRecoHitInformation;


#ifndef __CINT__
    template<class HitType, class ...Args>
    bool addHit(HitType* hit, Args&& ... params) const
    {
      if (hasHit(hit)) {
        return false;
      }

      StoreArray<RecoHitInformation> recoHitInformations(m_storeArrayNameOfRecoHitInformation);
      RecoHitInformation* newRecoHitInformation = recoHitInformations.appendNew(hit, params...);

      addHitWithHitInformation(hit, newRecoHitInformation);

      return true;
    }
#endif

    template <class HitType>
    bool setReconstructedArcLength(HitType* hit, double travelS) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformation(hit);
      if (recoHitInformation == nullptr)
        return false;
      else
        recoHitInformation->setReconstructedArcLength(travelS);

      return true;
    }

    template<class HitType>
    void mapOnHits(const std::string& storeArrayNameOfHits,
                   std::function<void(RecoHitInformation&, HitType* const)> mapFunction,
                   std::function<bool(const RecoHitInformation&, const HitType* const)> pickFunction)
    {
      RelationVector<RecoHitInformation> relatedHitInformation = getRelationsTo<RecoHitInformation>
                                                                 (m_storeArrayNameOfRecoHitInformation);

      for (RecoHitInformation& hitInformation : relatedHitInformation) {
        HitType* const hit = hitInformation.getRelatedTo<HitType>(storeArrayNameOfHits);
        assert(hit != nullptr);
        if (pickFunction(hitInformation, hit)) {
          mapFunction(hitInformation, hit);
        }
      }
    }

    template<class HitType>
    void mapOnHits(const std::string& storeArrayNameOfHits,
                   std::function<void(const RecoHitInformation&, const HitType* const)> mapFunction,
                   std::function<bool(const RecoHitInformation&, const HitType* const)> pickFunction) const
    {
      RelationVector<RecoHitInformation> relatedHitInformation = getRelationsTo<RecoHitInformation>
                                                                 (m_storeArrayNameOfRecoHitInformation);

      for (const RecoHitInformation& hitInformation : relatedHitInformation) {
        const HitType* const hit = hitInformation.getRelatedTo<HitType>(storeArrayNameOfHits);
        assert(hit != nullptr);
        if (pickFunction(hitInformation, hit)) {
          mapFunction(hitInformation, hit);
        }
      }
    }

    template<class HitType>
    void mapOnHits(const std::string& storeArrayNameOfHits,
                   std::function<void(RecoHitInformation&, HitType* const)> mapFunction)
    {
      mapOnHits<HitType>(storeArrayNameOfHits, mapFunction, [](const RecoHitInformation&, const HitType * const) -> bool { return true; });
    }

    template<class HitType>
    void mapOnHits(const std::string& storeArrayNameOfHits,
                   std::function<void(const RecoHitInformation&, const HitType* const)> mapFunction) const
    {
      mapOnHits<HitType>(storeArrayNameOfHits, mapFunction, [](const RecoHitInformation&, const HitType * const) -> bool { return true; });
    }

    template <class HitType>
    bool addHitWithHitInformation(HitType* hit, RecoHitInformation* recoHitInformation) const
    {
      hit->addRelationTo(this);
      addRelationTo(recoHitInformation);

      return true;
    }

    template<class HitType>
    RecoHitInformation* getRecoHitInformation(HitType* hit, const std::string& storeArrayNameOfHits) const
    {
      RelationVector<RecoHitInformation> relatedHitInformationToRecoTrack = getRelationsTo<RecoHitInformation>
          (m_storeArrayNameOfRecoHitInformation);

      for (RecoHitInformation& recoHitInformation : relatedHitInformationToRecoTrack) {
        if (recoHitInformation.getRelatedTo<HitType>(storeArrayNameOfHits) == hit) {
          return &recoHitInformation;
        }
      }

      return nullptr;
    }

    template <class HitType>
    unsigned int getNumberOfHits(const std::string& storeArrayNameOfHits) const
    {
      return getRelationsFrom<HitType>(storeArrayNameOfHits).size();
    }

    template<class HitType>
    std::vector<HitType*> getSortedHitList(const std::string& storeArrayNameOfHits) const
    {
      const RelationVector<RecoHitInformation>& relatedHitInformation = getRelationsTo<RecoHitInformation>
          (m_storeArrayNameOfRecoHitInformation);

      std::vector<const RecoHitInformation*> relatedHitInformationAsVector;
      relatedHitInformationAsVector.reserve(relatedHitInformation.size());
      for (const RecoHitInformation& hitInformation : relatedHitInformation) {
        relatedHitInformationAsVector.push_back(&hitInformation);
      }
      std::sort(relatedHitInformationAsVector.begin(), relatedHitInformationAsVector.end(), [](const RecoHitInformation * a,
      const RecoHitInformation * b) -> bool {
        return a->getReconstructedArcLength() < b->getReconstructedArcLength();
      });

      std::vector<HitType*> hitList;
      hitList.reserve(relatedHitInformationAsVector.size());
      for (const RecoHitInformation* hitInformation : relatedHitInformationAsVector) {
        HitType* relatedHit = hitInformation->getRelatedTo<HitType>(storeArrayNameOfHits);
        if (relatedHit != nullptr) {
          hitList.push_back(relatedHit);
        }
      }

      return hitList;
    }

    // Maybe an iterator would be better!
    template<class HitType>
    std::vector<HitType*> getHitList(const std::string& storeArrayNameOfHits) const
    {
      RelationVector<HitType> relatedHits = getRelationsFrom<HitType>(storeArrayNameOfHits);
      std::vector<HitType*> hitList;
      hitList.reserve(relatedHits.size());
      for (HitType& hit : relatedHits) {
        hitList.push_back(&hit);
      }

      return hitList;
    }

    //-----------------------------------------------------------------------------------
    /** Making this class a ROOT class.*/
    ClassDef(RecoTrack, 2);
  };
}

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

#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>

#include <tracking/dataobjects/RecoHitInformation.h>
#include <framework/datastore/RelationsObject.h>

#include <genfit/Track.h>

#include <vector>
#include <string>

#include <framework/datastore/StoreArray.h>

#include <framework/gearbox/Const.h>

namespace genfit {
  class AbsKalmanFitter;
}

namespace Belle2 {

  /** This is the Reconstruction Event-Data Model Track.
   *
   *  ///FIXME clean this comment!
   *  This class collects hits, performs fits and saves the Track parameters.
   *  Note: This class is still experimental.
   *  Totally missing:
   *   - Fitting functionality -> relate to genfit::Track
   *
   *    TODO: Covariance matrix
   *    TODO: Hits are now only added to the recoTrack - not to the genfit::Track. We have to take care of this!
   */
  class RecoTrack : public RelationsInterface<genfit::Track> {
  public:
    /**
     * Copy the definitions from the RecoHitInformation to this class,
     * to access it from the outside.
     */
    typedef RecoHitInformation::RightLeftInformation RightLeftInformation;
    typedef RecoHitInformation::TrackingDetector TrackingDetector;
    typedef RecoHitInformation::OriginTrackFinder OriginTrackFinder;
    typedef RecoHitInformation::UsedCDCHit UsedCDCHit;
    typedef RecoHitInformation::UsedPXDHit UsedPXDHit;
    typedef RecoHitInformation::UsedSVDHit UsedSVDHit;

    /**
     * Empty constructor for ROOT.
     */
    RecoTrack() :
      m_charge(),
      m_storeArrayNameOfCDCHits(),
      m_storeArrayNameOfSVDHits(),
      m_storeArrayNameOfPXDHits(),
      m_storeArrayNameOfRecoHitInformation(),
      m_lastFitSucessfull(false)
    {}

    /**
       * Construct a RecoTrack with the given helix parameters and the given names for the hits.
       * If you do not provide information for the hit store array names, the standard parameters are used.
       * @param position A position on the helix. Only the perigee of the helix will be saved.
       * @param momentum The momentum of the helix on the given position.
       * @param charge The charge of the helix
       * @param storeArrayNameOfCDCHits The name of the store array where the related cdc hits are stored.
       * @param storeArrayNameOfSVDHits The name of the store array where the related svd hits are stored.
       * @param storeArrayNameOfPXDHits The name of the store array where the related pxd hits are stored.
       * @param storeArrayNameOfRecoHitInformation The name of the store array where the related hit information are stored.
       */
    RecoTrack(const TVector3& position, const TVector3& momentum, const short int charge,
              const std::string& storeArrayNameOfCDCHits = "CDCHits",
              const std::string& storeArrayNameOfSVDHits = "SVDClusters",
              const std::string& storeArrayNameOfPXDHits = "PXDClusters",
              const std::string& storeArrayNameOfRecoHitInformation = "RecoHitInformations");

    /**
     * Create a reco track from a genfit::TrackCand and save it to the given store array.
     * @param trackCand The genfit::TrackCand from which to create the new object.
     * @param storeArrayNameOfRecoTracks The store array where the new object should be saved.
     * @param storeArrayNameOfCDCHits The name of the store array where the related cdc hits are stored.
     * @param storeArrayNameOfSVDHits The name of the store array where the related svd hits are stored.
     * @param storeArrayNameOfPXDHits The name of the store array where the related pxd hits are stored.
     * @param storeArrayNameOfRecoHitInformation The name of the store array where the related hit information are stored.
     * @return The newly created reco track.
     */
    static RecoTrack* createFromTrackCand(const genfit::TrackCand* trackCand,
                                          const std::string& storeArrayNameOfRecoTracks = "RecoTracks",
                                          const std::string& storeArrayNameOfCDCHits = "CDCHits",
                                          const std::string& storeArrayNameOfSVDHits = "SVDClusters",
                                          const std::string& storeArrayNameOfPXDHits = "PXDClusters",
                                          const std::string& storeArrayNameOfRecoHitInformation = "RecoHitInformations"
                                         );

    /**
     * Create a genfit::TrackCand out of this reco track and copy all information to the track candidate.
     * @return
     */
    genfit::TrackCand* createGenfitTrackCand() const;

    /**
     * Adds a cdc hit with the given information to the reco track.
     * You only have to provide the hit and the arc length, all other parameters have default value.
     * @param cdcHit The pointer to a stored CDCHit in the store array you provided earlier, which you want to add.
     * @param sortingParameter The arc length of the hit. The arc length is - by our definition - between -pi and pi.
     * @param rightLeftInformation
     * @param foundByTrackFinder
     * @return True if the hit was not already added to the track.
     */
    bool addCDCHit(UsedCDCHit* cdcHit, const double sortingParameter,
                   RightLeftInformation rightLeftInformation = RightLeftInformation::undefinedRightLeftInformation,
                   OriginTrackFinder foundByTrackFinder = OriginTrackFinder::undefinedTrackFinder) const
    {
      return addHit(cdcHit, rightLeftInformation, foundByTrackFinder, sortingParameter);
    }

    /**
     * Adds a pxd hit with the given information to the reco track.
     * You only have to provide the hit and the arc length, all other parameters have default value.
     * @param pxdHit The pointer to a stored PXDHit in the store array you provided earlier, which you want to add.
     * @param sortingParameter The arc length of the hit. The arc length is - by our definition - between -pi and pi.
     * @param foundByTrackFinder
     * @return True if the hit was not already added to the track.
     */
    bool addPXDHit(UsedPXDHit* pxdHit, const double sortingParameter,
                   OriginTrackFinder foundByTrackFinder = OriginTrackFinder::undefinedTrackFinder) const
    {
      return addHit(pxdHit, RightLeftInformation::undefinedRightLeftInformation, foundByTrackFinder, sortingParameter);
    }

    /**
     * Adds a svd hit with the given information to the reco track.
     * You only have to provide the hit and the arc length, all other parameters have default value.
     * @param svdHit The pointer to a stored SVDHit in the store array you provided earlier, which you want to add.
     * @param sortingParameter The arc length of the hit. The arc length is - by our definition - between -pi and pi.
     * @param foundByTrackFinder
     * @return True if the hit was not already added to the track.
     */
    bool addSVDHit(UsedSVDHit* svdHit, const double sortingParameter,
                   OriginTrackFinder foundByTrackFinder = OriginTrackFinder::undefinedTrackFinder) const
    {
      return addHit(svdHit, RightLeftInformation::undefinedRightLeftInformation, foundByTrackFinder, sortingParameter);
    }

    /**
     * Return the reco hit information for a given cdc hit.
     * @param cdcHit
     * @return the reco hit information.
     */
    RecoHitInformation* getRecoHitInformation(UsedCDCHit* cdcHit) const
    {
      return getRecoHitInformation(cdcHit, m_storeArrayNameOfCDCHits);
    }

    /**
     * Return the reco hit information for a given svd hit.
     * @param svdHit
     * @return the reco hit information.
     */
    RecoHitInformation* getRecoHitInformation(UsedSVDHit* cdcHit) const
    {
      return getRecoHitInformation(cdcHit, m_storeArrayNameOfSVDHits);
    }

    /**
     * Return the reco hit information for a given pxd hit.
     * @param pxdHit
     * @return the reco hit information.
     */
    RecoHitInformation* getRecoHitInformation(UsedPXDHit* cdcHit) const
    {
      return getRecoHitInformation(cdcHit, m_storeArrayNameOfPXDHits);
    }

    // Hits Information Questioning
    /**
     * Return the tracking detector of a given hit (every type).
     * @param hit
     * @return the tracking detector.
     */
    template <class HitType>
    TrackingDetector getTrackingDetector(HitType* hit) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformation(hit);
      if (recoHitInformation == nullptr)
        return TrackingDetector::invalidTrackingDetector;
      else
        return recoHitInformation->getTrackingDetector();
    }

    /**
     * Return the right left information of a given hit (every type).
     * @param hit
     * @return the right left information
     */
    template <class HitType>
    RightLeftInformation getRightLeftInformation(HitType* hit) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformation(hit);
      if (recoHitInformation == nullptr)
        return RightLeftInformation::invalidRightLeftInformation;
      else
        return recoHitInformation->getRightLeftInformation();
    }

    /**
     * Return the found by track finder flag for the given hit (every type)
     * @param hit
     * @return the found by track finder flag
     */
    template <class HitType>
    OriginTrackFinder getFoundByTrackFinder(HitType* hit) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformation(hit);
      if (recoHitInformation == nullptr)
        return OriginTrackFinder::invalidTrackFinder;
      else
        return recoHitInformation->getFoundByTrackFinder();
    }

    /**
     * Return the sorting parameter for a given hit (every type)
     * @param hit
     * @return the soring paramter
     */
    template <class HitType>
    double getSortingParameter(HitType* hit) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformation(hit);
      if (recoHitInformation == nullptr)
        return std::nan("");
      else
        return recoHitInformation->getSortingParameter();
    }

    /**
     * Set the right left information.
     * @param hit
     * @param rightLeftInformation
     * @return
     */
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

    /**
     * Set the found by track finder flag
     * @param hit
     * @param originTrackFinder
     * @return
     */
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

    /**
     * Set the sorting paramter
     * @param hit
     * @param sortingParameter
     * @return
     */
    template <class HitType>
    bool setSortingParameter(HitType* hit, double sortingParameter) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformation(hit);
      if (recoHitInformation == nullptr)
        return false;
      else
        recoHitInformation->setSortingParameter(sortingParameter);

      return true;
    }

    // Hits Added Questioning
    /**
     * Returns true if the track has cdc hits.
     * @return
     */
    bool hasCDCHits() const
    {
      return getRelatedFrom<UsedCDCHit>(m_storeArrayNameOfCDCHits) != nullptr;
    }

    /**
     * Returns true if the track has svd hits.
     * @return
     */
    bool hasSVDHits() const
    {
      return getRelatedFrom<Belle2::RecoTrack::UsedSVDHit>(m_storeArrayNameOfSVDHits) != nullptr;
    }

    /**
     * Returns true if the track has pxd hits.
     * @return
     */
    bool hasPXDHits() const
    {
      return getRelatedFrom<Belle2::RecoTrack::UsedPXDHit>(m_storeArrayNameOfPXDHits) != nullptr;
    }

    /**
     * Returns true if the given hit is in the track.
     * @param hit
     * @return
     */
    template <class HitType>
    bool hasHit(HitType* hit) const
    {
      const RelationVector<RecoTrack>& relatedTracksToHit = hit->template getRelationsTo<RecoTrack>(getArrayName());
      return std::find_if(relatedTracksToHit.begin(), relatedTracksToHit.end(), [this](const RecoTrack & recoTrack) {
        return &recoTrack == this;
      }) != relatedTracksToHit.end();
    }

    // Hits Questioning
    /**
     * Return the number of cdc hits.
     * @return
     */
    unsigned int getNumberOfCDCHits() const
    {
      return getNumberOfHitsOfGivenType<UsedCDCHit>(m_storeArrayNameOfCDCHits);
    }

    /**
     * Return the number of svd hits.
     * @return
     */
    unsigned int getNumberOfSVDHits() const
    {
      return getNumberOfHitsOfGivenType<Belle2::RecoTrack::UsedSVDHit>(m_storeArrayNameOfSVDHits);
    }

    /**
     * Return the number of pxd hits.
     * @return
     */
    unsigned int getNumberOfPXDHits() const
    {
      return getNumberOfHitsOfGivenType<Belle2::RecoTrack::UsedPXDHit>(m_storeArrayNameOfPXDHits);
    }

    /**
     * Return the number of cdc + svd + pxd hits.
     * @return
     */
    unsigned int getNumberOfTotalHits() const
    {
      return getNumberOfCDCHits() + getNumberOfPXDHits() + getNumberOfSVDHits();
    }

    /**
     * Return an unsorted list of cdc hits.
     * @return
     */
    std::vector<Belle2::RecoTrack::UsedCDCHit*> getCDCHitList() const
    {
      return getHitList<UsedCDCHit>(m_storeArrayNameOfCDCHits);
    }

    /**
     * Return an unsorted list of svd hits.
     * @return
     */
    std::vector<Belle2::RecoTrack::UsedSVDHit*> getSVDHitList() const
    {
      return getHitList<Belle2::RecoTrack::UsedSVDHit>(m_storeArrayNameOfSVDHits);
    }

    /**
     * Return an unsorted list of pxd hits.
     * @return
     */
    std::vector<Belle2::RecoTrack::UsedPXDHit*> getPXDHitList() const
    {
      return getHitList<Belle2::RecoTrack::UsedPXDHit>(m_storeArrayNameOfPXDHits);
    }

    /**
     * Return a sorted list of cdc hits. Sorted by the sortingParameter.
     * @return
     */
    std::vector<Belle2::RecoTrack::UsedCDCHit*> getSortedCDCHitList() const
    {
      return getSortedHitList<UsedCDCHit>(m_storeArrayNameOfCDCHits);
    }

    /**
     * Return a sorted list of svd hits. Sorted by the sortingParameter.
     * @return
     */
    std::vector<Belle2::RecoTrack::UsedSVDHit*> getSortedSVDHitList() const
    {
      return getSortedHitList<Belle2::RecoTrack::UsedSVDHit>(m_storeArrayNameOfSVDHits);
    }

    /**
     * Return a sorted list of pxd hits. Sorted by the sortingParameter.
     * @return
     */
    std::vector<Belle2::RecoTrack::UsedPXDHit*> getSortedPXDHitList() const
    {
      return getSortedHitList<Belle2::RecoTrack::UsedPXDHit>(m_storeArrayNameOfPXDHits);
    }

    // Helix Stuff
    /**
     * Return the position stored in the reco track.
     * @return
     */
    TVector3 getPosition() const
    {
      const TVectorD& seed = getStateSeed();
      return TVector3(seed(0), seed(1), seed(2));
    }

    /**
     * Return the momentum stored in the reco track.
     * @return
     */
    TVector3 getMomentum() const
    {
      const TVectorD& seed = getStateSeed();
      return TVector3(seed(3), seed(4), seed(5));
    }

    /**
     * Return the charge stored in the reco track.
     * @return
     */
    short int getCharge() const
    {
      return m_charge;
    }

    // Hit Pattern stuff
    /**
     * Return the hit pattern for the cdc hits.
     * @param pseudoCharge
     * @return
     * TODO: For this the sorting parameter has to be the travel S. We should think about this.
     */
    HitPatternCDC getHitPatternCDC(const short pseudoCharge) const
    {
      HitPatternCDC hitPatternCDC;

      mapOnHits<UsedCDCHit>(m_storeArrayNameOfCDCHits, [&hitPatternCDC](const RecoHitInformation&, const UsedCDCHit * const hit) -> void {
        // I need to initialize a WireID with the ID from the CDCHit to get the continuous layer ID.
        WireID wireID(hit->getID());
        // Then I set the corresponding layer in the hit pattern.
        hitPatternCDC.setLayer(wireID.getICLayer());
      }, [&pseudoCharge](const RecoHitInformation & hitInformation, const UsedCDCHit * const) -> bool {
        // Little trick: if we want the first half, we want the s to be 0 <= s <= pi,
        // if we want the second half, we want the s to be -pi <= s <= 0. Because -pi <= s <= pi is assured
        // by the RecoHitInformation, we only have to test of s > 0 or s < 0. For speed we test if s > 0 or -s > 0.
        return pseudoCharge * hitInformation.getSortingParameter() > 0;
      });
      return hitPatternCDC;
    }

    /**
     * Return the hit pattern for the vxd hits.
     * @param pseudoCharge
     * @return
     * TODO: For this the sorting parameter has to be the travel S. We should think about this.
     */
    HitPatternVXD getHitPatternVXD(const short /*pseudoCharge*/) const
    {
      HitPatternVXD hitPatternVXD;
      /* TODO */
      return hitPatternVXD;
    }

#ifndef __CINT__
    /**
     * Fit the track with the given abs fitter from genfit.
     * @param fitter the preinitialized fitter
     * @param pdgCodeForFit the pdg code we use for fitting. If you set the wrong charge, the method will turn it the other way round.
     * @param resortHits Whether the fitter should try to resort the hits
     */
    void fit(const std::shared_ptr<genfit::AbsKalmanFitter>& fitter, int pdgCodeForFit, bool resortHits);
#endif

    /**
     * Was the last fit sucessful?
     * @return
     */
    bool wasLastFitSucessfull() const
    {
      return m_lastFitSucessfull;
    }

    const std::string& getStoreArrayNameOfCDCHits() const
    {
      return m_storeArrayNameOfCDCHits;
    }

    const std::string& getStoreArrayNameOfSVDHits() const
    {
      return m_storeArrayNameOfSVDHits;
    }

    const std::string& getStoreArrayNameOfPXDHits() const
    {
      return m_storeArrayNameOfPXDHits;
    }

    const std::string& getStoreArrayNameOfRecoHitInformation() const
    {
      return m_storeArrayNameOfRecoHitInformation;
    }

  private:
    unsigned short int m_charge; /**< Storage for the charge. All other helix parameters are saved in the genfit::Track */
    std::string m_storeArrayNameOfCDCHits; /**< Store array of added cdc hits */
    std::string m_storeArrayNameOfSVDHits; /**< Store array of added svd hits */
    std::string m_storeArrayNameOfPXDHits; /**< Store array of added pxd hits */
    std::string m_storeArrayNameOfRecoHitInformation;  /**< Store array of added reco hit information */
    bool m_lastFitSucessfull; /**< Bool if the last fit was sucessfull */


#ifndef __CINT__
    /**
     * Add a generic hit with the given parameters for the reco hit information.
     * @param hit a generic hit.
     * @param params for the constructor of the reco hit information.
     * @return
     */
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

  public:
    /**
     * Call a function on all hits of the given type in the store array, that are related to this track.
     * @param storeArrayNameOfHits
     * @param mapFunction
     * @param pickFunction Use only those hits where the function returns true.
     */
    template<class HitType>
    void mapOnHits(const std::string& storeArrayNameOfHits,
                   std::function<void(RecoHitInformation&, HitType* const)> mapFunction,
                   std::function<bool(const RecoHitInformation&, const HitType* const)> pickFunction)
    {
      RelationVector<RecoHitInformation> relatedHitInformation = getRelationsTo<RecoHitInformation>
                                                                 (m_storeArrayNameOfRecoHitInformation);

      for (RecoHitInformation& hitInformation : relatedHitInformation) {
        HitType* const hit = hitInformation.getRelatedTo<HitType>(storeArrayNameOfHits);
        if (hit != nullptr && pickFunction(hitInformation, hit)) {
          mapFunction(hitInformation, hit);
        }
      }
    }

    /**
     * Call a function on all hits of the given type in the store array, that are related to this track. Const version.
     * @param storeArrayNameOfHits
     * @param mapFunction
     * @param pickFunction Use only those hits where the function returns true.
     */
    template<class HitType>
    void mapOnHits(const std::string& storeArrayNameOfHits,
                   std::function<void(const RecoHitInformation&, const HitType* const)> mapFunction,
                   std::function<bool(const RecoHitInformation&, const HitType* const)> pickFunction) const
    {
      RelationVector<RecoHitInformation> relatedHitInformation = getRelationsTo<RecoHitInformation>
                                                                 (m_storeArrayNameOfRecoHitInformation);

      for (const RecoHitInformation& hitInformation : relatedHitInformation) {
        const HitType* const hit = hitInformation.getRelatedTo<HitType>(storeArrayNameOfHits);
        if (hit != nullptr && pickFunction(hitInformation, hit)) {
          mapFunction(hitInformation, hit);
        }
      }
    }

    /**
     * Call a function on all hits of the given type in the store array, that are related to this track.
     * @param storeArrayNameOfHits
     * @param mapFunction
     */
    template<class HitType>
    void mapOnHits(const std::string& storeArrayNameOfHits,
                   std::function<void(RecoHitInformation&, HitType* const)> mapFunction)
    {
      mapOnHits<HitType>(storeArrayNameOfHits, mapFunction, [](const RecoHitInformation&, const HitType * const) -> bool { return true; });
    }

    /**
     * Call a function on all hits of the given type in the store array, that are related to this track. Const version.
     * @param storeArrayNameOfHits
     * @param mapFunction
     */
    template<class HitType>
    void mapOnHits(const std::string& storeArrayNameOfHits,
                   std::function<void(const RecoHitInformation&, const HitType* const)> mapFunction) const
    {
      mapOnHits<HitType>(storeArrayNameOfHits, mapFunction, [](const RecoHitInformation&, const HitType * const) -> bool { return true; });
    }

  private:
    /**
     * Add a generic hit with the given hit information.
     * @param hit
     * @param recoHitInformation
     * @return
     */
    template <class HitType>
    bool addHitWithHitInformation(HitType* hit, RecoHitInformation* recoHitInformation) const
    {
      hit->addRelationTo(this);
      addRelationTo(recoHitInformation);

      return true;
    }

    /**
     * Return the reco hit information for a generic hit from the storeArray.
     * @param hit
     * @param storeArrayNameOfHits
     * @return
     */
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

    /**
     * Calculate the time seed before fitting.
     * @param particleWithPDGCode the particle we use for calculating the time seed.
     */
    void calculateTimeSeed(TParticlePDG* particleWithPDGCode);

    /**
     * Get the number of hits for thee given hit type in the store array that are related to this track.
     * @param storeArrayNameOfHits
     * @return
     */
    template <class HitType>
    unsigned int getNumberOfHitsOfGivenType(const std::string& storeArrayNameOfHits) const
    {
      return getRelationsFrom<HitType>(storeArrayNameOfHits).size();
    }

    /**
     * Return a sorted list of hits of the given type in the store array that are related to this track.
     * @param storeArrayNameOfHits
     * @return
     */
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
        return a->getSortingParameter() < b->getSortingParameter();
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
    /**
     * Return an unsorted list of hits of the given type in the store array that are related to this track.
     * @param storeArrayNameOfHits
     * @return
     */
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
    ClassDef(RecoTrack, 3);
  };
}

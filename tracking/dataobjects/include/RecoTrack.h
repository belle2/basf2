/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Nils Braun, Thomas Hauth                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/core/FrameworkExceptions.h>

#include <genfit/Track.h>

#include <tracking/dataobjects/RecoHitInformation.h>

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace genfit {
  class AbsFitter;
  class TrackCand;
  class AbsTrackRep;
}

namespace Belle2 {

  template <class T>
  class StoreArray;

  class RecoTrackGenfitAccess;

  BELLE2_DEFINE_EXCEPTION(NoTrackFitResult, "No track fit result available for this hit (e.g. DAF has removed it).")

  /** This is the Reconstruction Event-Data Model Track.
   *
   * This class collects hits, saves the track parameters and can be used with a TrackFitter to perform
   * fits to the hits. It can be created from a genfit::TrackCand and converted to a genfit::Track,
   * but it is better to use this class directly in the modules, because it offers a more datastore-suited interface.
   *
   * The RecoTrack itself does only store a genfit::Track internally. All hit content is stored as a relation to
   * the hits. For each of these relations to a detector hit, there is also a relation to a RecoHitInformation,
   * to store additional information. However, the user does not need to access these relation by himself,
   * but can use the accessor functions of the RecoTrack.
   *
   * Typically, a RecoTrack object is created with a position and momentum seed and a charge.
   *
   *    RecoTrack recoTrack(position, momentum, charge, ...)
   *
   * Then, hits are added
   *
   *    recoTrack.addCDCHit(cdcHit, rlInformation, ...)
   *    ....
   *
   * After that, the hits can either be accessed:
   *
   *    recoTrack.getCDCHitList();
   *    recoTrack.getRightLeftInformation(cdcHit);
   *
   * or the track can be fitted:
   *
   *    TrackFitter fitter;
   *    fitter.fit(recoTrack);
   *
   * See also the TrackFitter class for possibilities to fit. After the track is fitted properly, the
   * hit points with measurements can be used to extrapolate the track
   *
   *    recoTrack.getHitPointsWithMeasurement();
   *
   * See the recoTrack.cc test for an overview on the hit information accessor methods.
   */
  class RecoTrack : public RelationsObject {
    /// The RecoTrackGenfitAccess need to access the genfit track (which is intended)!
    friend class RecoTrackGenfitAccess;

  private:
    /// Copy the definitions from the RecoHitInformation to this class.
    typedef RecoHitInformation::RightLeftInformation RightLeftInformation;
    /// Copy the definitions from the RecoHitInformation to this class.
    typedef RecoHitInformation::RecoHitDetector TrackingDetector;
    /// Copy the definitions from the RecoHitInformation to this class.
    typedef RecoHitInformation::OriginTrackFinder OriginTrackFinder;
    /// Copy the definitions from the RecoHitInformation to this class.
    typedef RecoHitInformation::UsedCDCHit UsedCDCHit;
    /// Copy the definitions from the RecoHitInformation to this class.
    typedef RecoHitInformation::UsedSVDHit UsedSVDHit;
    /// Copy the definitions from the RecoHitInformation to this class.
    typedef RecoHitInformation::UsedPXDHit UsedPXDHit;
    /// Copy the definitions from the RecoHitInformation to this class.
    typedef RecoHitInformation::UsedBKLMHit UsedBKLMHit;
    /// Copy the definitions from the RecoHitInformation to this class.
    typedef RecoHitInformation::UsedEKLMHit UsedEKLMHit;

  public:
    /**
     * Enum for the matching status of this reco track (set by the matching modules in the tracking package).
     */
    enum MatchingStatus {
      c_undefined,
      c_matched,
      c_clone,
      c_background,
      c_ghost
    };

    /**
     * Convenience method which registers all relations required to fully use
     * a RecoTrack. If you create a new RecoTrack StoreArray, call this method
     * in the initialize() method of your module. Note that the BKLM and EKLM
     * relations may not be registered because the KLM modules are loaded after
     * tracking; in this case, a second call of this method is required after
     * creation of the BKLM and EKLM hits store arrays.
     * @param recoTracks  Reference to the store array where the new RecoTrack list is located
     * @param cdcHitsStoreArrayName  name of the StoreArray holding the CDCHits lists
     * @param svdHitsStoreArrayName  name of the StoreArray holding the SVDClusters lists
     * @param pxdHitsStoreArrayName  name of the StoreArray holding the PXDClusters lists
     * @param bklmHitsStoreArrayName  name of the StoreArray holding the CDCHits lists
     * @param eklmHitsStoreArrayName  name of the StoreArray holding the CDCHits lists
     * @param recoHitInformationStoreArrayName  name of the StoreArray holding RecoHitInformation lists
     */
    static void registerRequiredRelations(
      StoreArray<RecoTrack>& recoTracks,
      std::string const& pxdHitsStoreArrayName = "",
      std::string const& svdHitsStoreArrayName = "",
      std::string const& cdcHitsStoreArrayName = "",
      std::string const& bklmHitsStoreArrayName = "",
      std::string const& eklmHitsStoreArrayName = "",
      std::string const& recoHitInformationStoreArrayName = "");

    /// Empty constructor for ROOT. Do not use!
    RecoTrack() { }

    /**
       * Construct a RecoTrack with the given seed helix parameters and the given names for the hits.
       * If you do not provide information for the hit store array names, the standard parameters are used.
       * @param seedPosition A position on the helix of the track seed. Only the perigee of the helix will be saved.
       * @param seedMomentum The seed momentum of the helix on the given position.
       * @param seedCharge The seed charge of the helix
       * @param storeArrayNameOfCDCHits The name of the store array where the related cdc hits are stored.
       * @param storeArrayNameOfSVDHits The name of the store array where the related svd hits are stored.
       * @param storeArrayNameOfPXDHits The name of the store array where the related pxd hits are stored.
       * @param storeArrayNameOfBKLMHits The name of the store array where the related pxd hits are stored.
       * @param storeArrayNameOfEKLMHits The name of the store array where the related pxd hits are stored.
       * @param storeArrayNameOfRecoHitInformation The name of the store array where the related hit information are stored.
       */
    RecoTrack(const TVector3& seedPosition, const TVector3& seedMomentum, const short int seedCharge,
              const std::string& storeArrayNameOfCDCHits = "",
              const std::string& storeArrayNameOfSVDHits = "",
              const std::string& storeArrayNameOfPXDHits = "",
              const std::string& storeArrayNameOfBKLMHits = "",
              const std::string& storeArrayNameOfEKLMHits = "",
              const std::string& storeArrayNameOfRecoHitInformation = "");

    /** Delete the copy construtr. */
    RecoTrack(const RecoTrack&) = delete;
    /** Delete the copy construtr. */
    RecoTrack& operator=(RecoTrack const&) = delete;

    /**
     * Create a reco track from a genfit::TrackCand and save it to the given store array.
     * @param trackCand The genfit::TrackCand from which to create the new object.
     * @param storeArrayNameOfRecoTracks The store array where the new object should be saved.
     * @param storeArrayNameOfCDCHits The name of the store array where the related cdc hits are stored.
     * @param storeArrayNameOfSVDHits The name of the store array where the related svd hits are stored.
     * @param storeArrayNameOfPXDHits The name of the store array where the related pxd hits are stored.
     * @param storeArrayNameOfBKLMHits The name of the store array where the related bklm hits are stored.
     * @param storeArrayNameOfEKLMHits The name of the store array where the related eklm hits are stored.
     * @param storeArrayNameOfRecoHitInformation The name of the store array where the related hit information are stored.
     * @param recreateSortingParameters The VXDTF does not set the sorting parameters correctly (they are all 0).
     *        This flag can be used to recover the parameters.
     * @return The newly created reco track.
     * @todo Let the track finders determine the cov seed.
     */
    static RecoTrack* createFromTrackCand(const genfit::TrackCand& trackCand,
                                          const std::string& storeArrayNameOfRecoTracks = "",
                                          const std::string& storeArrayNameOfCDCHits = "",
                                          const std::string& storeArrayNameOfSVDHits = "",
                                          const std::string& storeArrayNameOfPXDHits = "",
                                          const std::string& storeArrayNameOfBKLMHits = "",
                                          const std::string& storeArrayNameOfEKLMHits = "",
                                          const std::string& storeArrayNameOfRecoHitInformation = "",
                                          const bool recreateSortingParameters = false
                                         );

    /**
     * Create a genfit::TrackCand out of this reco track and copy all information to the track candidate.
     */
    genfit::TrackCand createGenfitTrackCand() const;

    /**
     * Append a new RecoTrack to the given store array and copy its general properties, but not the hits themself.
     * The position, momentum, charge etc. are set to the given parameters.
     */
    RecoTrack* copyToStoreArrayUsing(StoreArray<RecoTrack>& storeArray, const TVector3& position,
                                     const TVector3& momentum, short charge,
                                     const TMatrixDSym& covariance, double timeSeed) const;

    /**
     * Append a new RecoTrack to the given store array and copy its general properties, but not the hits themself.
     * The position, momentum and charge are set to the seed values of this reco track.
     */
    RecoTrack* copyToStoreArrayUsingSeeds(StoreArray<RecoTrack>& storeArray) const;

    /**
     * Append a new RecoTrack to the given store array and copy its general properties, but not the hits themself.
     * The position, momentum and charge are set to the seed values of this reco track, if it was not fitted
     * or to the values at the first hit.
     */
    RecoTrack* copyToStoreArray(StoreArray<RecoTrack>& storeArray) const;

    /**
     * Add all hits from another RecoTrack to this RecoTrack.
     * @param recoTrack Pointer to the RecoTrack where the hits are copied from
     * @param sortingParameterOffset This number will be added to the sortingParameter of all hits copied
     *        from recoTrack. Set this to (largest sorting parameter) + 1 in order to add hits at the end of
     *        this reco track.
     * @param reversed: add the hits in a reversed order - each sorting parameter is set to
     *        maximal sorting parameter - sorting parameter + offset
     * @param minimalWeight: if set, do only copy hits with a weight above this (if fitted already with the DAF).
     * @return The number of hits copied.
     */
    size_t addHitsFromRecoTrack(const RecoTrack* recoTrack, unsigned int sortingParameterOffset = 0,
                                bool reversed = false, boost::optional<double> optionalMinimalWeight = boost::none);

    /**
     * Adds a cdc hit with the given information to the reco track.
     * You only have to provide the hit and the sorting parameter, all other parameters have default value.
     * @param cdcHit The pointer to a stored CDCHit in the store array you provided earlier, which you want to add.
     * @param sortingParameter The index of the hit. It starts with 0 with the first hit.
     * @param rightLeftInformation The right left information (if you know it).
     * @param foundByTrackFinder Which track finder has found the hit?
     * @return True if the hit was not already added to the track.
     */
    bool addCDCHit(const UsedCDCHit* cdcHit, const unsigned int sortingParameter,
                   RightLeftInformation rightLeftInformation = RightLeftInformation::c_undefinedRightLeftInformation,
                   OriginTrackFinder foundByTrackFinder = OriginTrackFinder::c_undefinedTrackFinder)
    {
      return addHit(cdcHit, rightLeftInformation, foundByTrackFinder, sortingParameter);
    }

    /**
     * Adds a pxd hit with the given information to the reco track.
     * You only have to provide the hit and the sorting parameter, all other parameters have default value.
     * @param pxdHit The pointer to a stored PXDHit/Cluster in the store array you provided earlier, which you want to add.
     * @param sortingParameter The index of the hit. It starts with 0 with the first hit.
     * @param foundByTrackFinder Which track finder has found the hit?
     * @return True if the hit was not already added to the track.
     */
    bool addPXDHit(const UsedPXDHit* pxdHit, const unsigned int sortingParameter,
                   OriginTrackFinder foundByTrackFinder = OriginTrackFinder::c_undefinedTrackFinder)
    {
      return addHit(pxdHit, foundByTrackFinder, sortingParameter);
    }

    /**
     * Adds a svd hit with the given information to the reco track.
     * You only have to provide the hit and the sorting parameter, all other parameters have default value.
     * @param svdHit The pointer to a stored SVDHit in the store array you provided earlier, which you want to add.
     * @param sortingParameter The index of the hit. It starts with 0 with the first hit.
     * @param foundByTrackFinder Which track finder has found the hit?
     * @return True if the hit was not already added to the track.
     */
    bool addSVDHit(const UsedSVDHit* svdHit, const unsigned int sortingParameter,
                   OriginTrackFinder foundByTrackFinder = OriginTrackFinder::c_undefinedTrackFinder)
    {
      return addHit(svdHit, foundByTrackFinder, sortingParameter);
    }

    /**
     * Adds a bklm hit with the given information to the reco track.
     * You only have to provide the hit and the sorting parameter, all other parameters have default value.
     * @param bklmHit The pointer to a stored BKLMHit in the store array you provided earlier, which you want to add.
     * @param sortingParameter The index of the hit. It starts with 0 with the first hit.
     * @param foundByTrackFinder Which track finder has found the hit?
     * @return True if the hit was not already added to the track.
     */
    bool addBKLMHit(const UsedBKLMHit* bklmHit, const unsigned int sortingParameter,
                    OriginTrackFinder foundByTrackFinder = OriginTrackFinder::c_undefinedTrackFinder)
    {
      return addHit(bklmHit, foundByTrackFinder, sortingParameter);
    }

    /**
     * Adds an eklm hit with the given information to the reco track.
     * You only have to provide the hit and the sorting parameter, all other parameters have default value.
     * @param bklmHit The pointer to a stored BKLMHit in the store array you provided earlier, which you want to add.
     * @param sortingParameter The index of the hit. It starts with 0 with the first hit.
     * @param foundByTrackFinder Which track finder has found the hit?
     * @return True if the hit was not already added to the track.
     */
    bool addEKLMHit(const UsedEKLMHit* eklmHit, const unsigned int sortingParameter,
                    OriginTrackFinder foundByTrackFinder = OriginTrackFinder::c_undefinedTrackFinder)
    {
      return addHit(eklmHit, foundByTrackFinder, sortingParameter);
    }

    /**
     * Return the reco hit information for a generic hit from the storeArray.
     * @param hit the hit to look for.
     * @return The connected RecoHitInformation or a nullptr when the hit is not connected to the track.
     */
    template<class HitType>
    RecoHitInformation* getRecoHitInformation(HitType* hit) const
    {
      RelationVector<RecoHitInformation> relatedHitInformationToHit = hit->template getRelationsFrom<RecoHitInformation>
      (m_storeArrayNameOfRecoHitInformation);

      for (RecoHitInformation& recoHitInformation : relatedHitInformationToHit) {
        if (recoHitInformation.getRelatedFrom<RecoTrack>(this->getArrayName()) == this) {
          return &recoHitInformation;
        }
      }

      return nullptr;
    }

    // Hits Information Questioning
    /// Return the tracking detector of a given hit (every type) or throws an exception of the hit is not related to the track.
    template <class HitType>
    TrackingDetector getTrackingDetector(const HitType* hit) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformationSafely(hit);
      return recoHitInformation->getTrackingDetector();
    }

    /// Return the right left information of a given hit (every type) or throws an exception of the hit is not related to the track.
    template <class HitType>
    RightLeftInformation getRightLeftInformation(const HitType* hit) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformationSafely(hit);
      return recoHitInformation->getRightLeftInformation();
    }

    /// Return the found by track finder flag for the given hit (every type) or throws an exception of the hit is not related to the track.
    template <class HitType>
    OriginTrackFinder getFoundByTrackFinder(const HitType* hit) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformationSafely(hit);
      return recoHitInformation->getFoundByTrackFinder();
    }

    /// Return the sorting parameter for a given hit (every type) or throws an exception of the hit is not related to the track.
    template <class HitType>
    unsigned int getSortingParameter(const HitType* hit) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformationSafely(hit);
      return recoHitInformation->getSortingParameter();
    }

    /// Set the right left information or throws an exception of the hit is not related to the track. Will set the dirty flag!
    template <class HitType>
    void setRightLeftInformation(const HitType* hit, RightLeftInformation rightLeftInformation)
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformationSafely(hit);
      recoHitInformation->setRightLeftInformation(rightLeftInformation);
      setDirtyFlag();
    }

    /// Set the found by track finder flag or throws an exception of the hit is not related to the track.
    template <class HitType>
    void setFoundByTrackFinder(const HitType* hit, OriginTrackFinder originTrackFinder)
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformationSafely(hit);
      recoHitInformation->setFoundByTrackFinder(originTrackFinder);
    }

    /// Set the sorting parameter or throws an exception of the hit is not related to the track. Will set the dirty flag!
    template <class HitType>
    void setSortingParameter(const HitType* hit, unsigned int sortingParameter)
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformationSafely(hit);
      recoHitInformation->setSortingParameter(sortingParameter);
      setDirtyFlag();
    }

    /** Get a pointer to the TrackPoint that was created from this hit. Can be a nullptr if no measurement was already created.
     * Please be aware that refitting may or may not recreate the track points and older pointers can be invalidated.
     * Also, pruning a RecoTrack will also delete most of the TrackPoints.
     */
    const genfit::TrackPoint* getCreatedTrackPoint(const RecoHitInformation* recoHitInformation) const;

    // Hits Added Questioning
    /// Returns true if the track has cdc hits.
    bool hasCDCHits() const { return getRelatedFrom<UsedCDCHit>(m_storeArrayNameOfCDCHits) != nullptr; }

    /// Returns true if the track has svd hits.
    bool hasSVDHits() const { return getRelatedFrom<UsedSVDHit>(m_storeArrayNameOfSVDHits) != nullptr; }

    /// Returns true if the track has pxd hits.
    bool hasPXDHits() const { return getRelatedFrom<UsedPXDHit>(m_storeArrayNameOfPXDHits) != nullptr; }

    /// Returns true if the track has bklm hits.
    bool hasBKLMHits() const { return getRelatedFrom<UsedBKLMHit>(m_storeArrayNameOfBKLMHits) != nullptr; }

    /// Returns true if the track has eklm hits.
    bool hasEKLMHits() const { return getRelatedFrom<UsedEKLMHit>(m_storeArrayNameOfEKLMHits) != nullptr; }

    /// Returns true if the given hit is in the track.
    template <class HitType>
    bool hasHit(const HitType* hit) const
    {
      const RelationVector<RecoTrack>& relatedTracksToHit = hit->template getRelationsTo<RecoTrack>(getArrayName());
      return std::find_if(relatedTracksToHit.begin(), relatedTracksToHit.end(), [this](const RecoTrack & recoTrack) {
        return &recoTrack == this;
      }) != relatedTracksToHit.end();
    }

    // Hits Questioning
    /// Return the number of cdc hits.
    unsigned int getNumberOfCDCHits() const { return getNumberOfHitsOfGivenType<UsedCDCHit>(m_storeArrayNameOfCDCHits); }

    /// Return the number of svd hits.
    unsigned int getNumberOfSVDHits() const { return getNumberOfHitsOfGivenType<UsedSVDHit>(m_storeArrayNameOfSVDHits); }

    /// Return the number of pxd hits.
    unsigned int getNumberOfPXDHits() const { return getNumberOfHitsOfGivenType<UsedPXDHit>(m_storeArrayNameOfPXDHits); }

    /// Return the number of bklm hits.
    unsigned int getNumberOfBKLMHits() const { return getNumberOfHitsOfGivenType<UsedBKLMHit>(m_storeArrayNameOfBKLMHits); }

    /// Return the number of eklm hits.
    unsigned int getNumberOfEKLMHits() const { return getNumberOfHitsOfGivenType<UsedEKLMHit>(m_storeArrayNameOfEKLMHits); }

    /// Return the number of cdc + svd + pxd + bklm + eklm hits.
    unsigned int getNumberOfTotalHits() const
    {
      return getNumberOfCDCHits() + getNumberOfPXDHits() +
             getNumberOfSVDHits() + getNumberOfBKLMHits() + getNumberOfEKLMHits();
    }

    /// Return the number of cdc + svd + pxd  hits.
    unsigned int getNumberOfTrackingHits() const
    {
      return getNumberOfCDCHits() + getNumberOfPXDHits() +
             getNumberOfSVDHits();
    }

    /// Return an unsorted list of cdc hits.
    std::vector<Belle2::RecoTrack::UsedCDCHit*> getCDCHitList() const { return getHitList<UsedCDCHit>(m_storeArrayNameOfCDCHits); }

    /// Return an unsorted list of svd hits.
    std::vector<Belle2::RecoTrack::UsedSVDHit*> getSVDHitList() const { return getHitList<UsedSVDHit>(m_storeArrayNameOfSVDHits); }

    /// Return an unsorted list of pxd hits.
    std::vector<Belle2::RecoTrack::UsedPXDHit*> getPXDHitList() const { return getHitList<UsedPXDHit>(m_storeArrayNameOfPXDHits); }

    /// Return an unsorted list of bklm hits.
    std::vector<Belle2::RecoTrack::UsedBKLMHit*> getBKLMHitList() const { return getHitList<UsedBKLMHit>(m_storeArrayNameOfBKLMHits); }

    /// Return an unsorted list of eklm hits.
    std::vector<Belle2::RecoTrack::UsedEKLMHit*> getEKLMHitList() const { return getHitList<UsedEKLMHit>(m_storeArrayNameOfEKLMHits); }

    /// Return a sorted list of cdc hits. Sorted by the sortingParameter.
    std::vector<Belle2::RecoTrack::UsedCDCHit*> getSortedCDCHitList() const { return getSortedHitList<UsedCDCHit>(m_storeArrayNameOfCDCHits); }

    /// Return a sorted list of svd hits. Sorted by the sortingParameter.
    std::vector<Belle2::RecoTrack::UsedSVDHit*> getSortedSVDHitList() const { return getSortedHitList<UsedSVDHit>(m_storeArrayNameOfSVDHits); }

    /// Return a sorted list of pxd hits. Sorted by the sortingParameter.
    std::vector<Belle2::RecoTrack::UsedPXDHit*> getSortedPXDHitList() const { return getSortedHitList<UsedPXDHit>(m_storeArrayNameOfPXDHits); }

    /// Return a sorted list of bklm hits. Sorted by the sortingParameter.
    std::vector<Belle2::RecoTrack::UsedBKLMHit*> getSortedBKLMHitList() const { return getSortedHitList<UsedBKLMHit>(m_storeArrayNameOfBKLMHits); }

    /// Return a sorted list of eklm hits. Sorted by the sortingParameter.
    std::vector<Belle2::RecoTrack::UsedEKLMHit*> getSortedEKLMHitList() const { return getSortedHitList<UsedEKLMHit>(m_storeArrayNameOfEKLMHits); }

    // Seed Helix Functionality
    /// Return the position seed stored in the reco track. ATTENTION: This is not the fitted position.
    TVector3 getPositionSeed() const
    {
      const TVectorD& seed = m_genfitTrack.getStateSeed();
      return TVector3(seed(0), seed(1), seed(2));
    }

    /// Return the momentum seed stored in the reco track. ATTENTION: This is not the fitted momentum.
    TVector3 getMomentumSeed() const
    {
      const TVectorD& seed = m_genfitTrack.getStateSeed();
      return TVector3(seed(3), seed(4), seed(5));
    }

    /// Return the state seed in the form posX, posY, posZ, momX, momY, momZ. ATTENTION: This is not the fitted state.
    const TVectorD& getStateSeed() const
    {
      return m_genfitTrack.getStateSeed();
    }

    /// Return the charge seed stored in the reco track. ATTENTION: This is not the fitted charge.
    short int getChargeSeed() const { return m_charge; }

    /// Return the time seed stored in the reco track. ATTENTION: This is not the fitted time.
    double getTimeSeed() const { return m_genfitTrack.getTimeSeed(); }

    /// Return the position, the momentum and the charge of the first measured state on plane or - if unfitted - the seeds.
    std::tuple<TVector3, TVector3, short> extractTrackState() const;

    /// Set the position and momentum seed of the reco track. ATTENTION: This is not the fitted position or momentum.
    void setPositionAndMomentum(const TVector3& positionSeed, const TVector3& momentumSeed)
    {
      m_genfitTrack.setStateSeed(positionSeed, momentumSeed);
      deleteFittedInformation();
    }

    /// Set the charge seed stored in the reco track. ATTENTION: This is not the fitted charge.
    void setChargeSeed(const short int chargeSeed)
    {
      m_charge = chargeSeed;
      deleteFittedInformation();
    }

    /// Set the time seed. ATTENTION: This is not the fitted time.
    void setTimeSeed(const double timeSeed)
    {
      m_genfitTrack.setTimeSeed(timeSeed);
      deleteFittedInformation();
    }

    /// Return the covariance matrix of the seed. ATTENTION: This is not the fitted covariance.
    const TMatrixDSym& getSeedCovariance() const { return m_genfitTrack.getCovSeed(); }

    /// Set the covariance of the seed. ATTENTION: This is not the fitted covariance.
    void setSeedCovariance(const TMatrixDSym& seedCovariance) { m_genfitTrack.setCovSeed(seedCovariance); }

    // Fitting
    /// Returns true if the last fit with the given representation was successful.
    bool wasFitSuccessful(const genfit::AbsTrackRep* representation = nullptr) const;

    /// Return the track fit status for the given representation or for the cardinal one. You are not allowed to modify or delete it!
    const genfit::FitStatus* getTrackFitStatus(const genfit::AbsTrackRep* representation = nullptr) const
    {
      checkDirtyFlag();
      return m_genfitTrack.getFitStatus(representation);
    }

    /// Check, if there is a fit status for the given representation or for the cardinal one.
    bool hasTrackFitStatus(const genfit::AbsTrackRep* representation = nullptr) const;

    /// Get a pointer to the cardinal track representation. You are not allowed to modify or delete it!
    genfit::AbsTrackRep* getCardinalRepresentation() const
    {
      checkDirtyFlag();
      return m_genfitTrack.getCardinalRep();
    }

    /// Return a list of track representations. You are not allowed to modify or delete them!
    const std::vector<genfit::AbsTrackRep*>& getRepresentations() const
    {
      checkDirtyFlag();
      return m_genfitTrack.getTrackReps();
    }

    /** Return an already created track representation of the given reco track for the PDG. You
     * are nowt allowed to modify this TrackRep! Will return nulltpr if a trackRep is not available
     * for the given pdgCode.
     *
     * @param pdgCode PDG code of the track representations, only positive PDG numbers are allowed
     */
    genfit::AbsTrackRep* getTrackRepresentationForPDG(int pdgCode);

    /**
     * Return a list of all RecoHitInformations associated with the RecoTrack. This is especially useful when
     * you want to iterate over all (fitted) hits in a track without caring whether its a CDC, VXD etc hit.
     * @param getSorted if true, the list of RecoHitInformations will be returned sorted by the Sorting parameter
     * in an ascending order. If false, the hits will be returned unsorted.
     */
    std::vector<RecoHitInformation*> getRecoHitInformations(bool getSorted = false) const;

    /** Return genfit's MeasuredStateOnPlane for the first hit in a fit
    * useful for extrapolation of measurements to other locations
    * Const version.
    */
    const genfit::MeasuredStateOnPlane& getMeasuredStateOnPlaneFromFirstHit(const genfit::AbsTrackRep* representation = nullptr) const;

    /** Return genfit's MeasuredStateOnPlane for the last hit in a fit
    * useful for extrapolation of measurements to other locations
    * Const version.
    */
    const genfit::MeasuredStateOnPlane& getMeasuredStateOnPlaneFromLastHit(const genfit::AbsTrackRep* representation = nullptr) const;

    /**
     * Return genfit's MeasuredStateOnPlane on plane for associated with one RecoHitInformation. The caller needs to ensure that
     * recoHitInfo->useInFit() is true and the a fit has been performed on the track, a.k.a. hasTrackFitStatus() == true
     */
    const genfit::MeasuredStateOnPlane& getMeasuredStateOnPlaneFromRecoHit(const RecoHitInformation* recoHitInfo,
        const genfit::AbsTrackRep* representation = nullptr) const;

    /** Return genfit's MasuredStateOnPlane, that is closest to the given point
     * useful for extrapolation of measurements other locations
     */
    const genfit::MeasuredStateOnPlane& getMeasuredStateOnPlaneClosestTo(const TVector3& closestPoint,
        const genfit::AbsTrackRep* representation = nullptr);

    /** Prune the genfit track, e.g. remove all track points with measurements, but the first and the last one.
      * Also, set the flags of the corresponding RecoHitInformation to pruned. Only to be used in the prune module.
      */
    void prune();

    /// Return a list of measurements and track points, which can be used e.g. to extrapolate. You are not allowed to modify or delete them!
    const std::vector<genfit::TrackPoint*>& getHitPointsWithMeasurement() const
    {
      checkDirtyFlag();
      return m_genfitTrack.getPointsWithMeasurement();
    }

    /// This returns true, if a hit was added after the last fit and measurement creation and a refit should be done.
    bool getDirtyFlag() const { return m_dirtyFlag; }

    /// Set to true, if you want to rebuild the measurements and do the fit independent on changes of the hit content.
    /**
     * You can use this setting if you want to use other measurement creators than before (probably non-default settings)
     * or different fitting algorithms.
     */
    void setDirtyFlag(const bool& dirtyFlag = true)
    {
      m_dirtyFlag = dirtyFlag;
      if (dirtyFlag) {
        deleteFittedInformation();
      }
    }

    // Store Array Names
    /// Name of the store array of the cdc hits.
    const std::string& getStoreArrayNameOfCDCHits() const { return m_storeArrayNameOfCDCHits; }

    /// Name of the store array of the svd hits.
    const std::string& getStoreArrayNameOfSVDHits() const { return m_storeArrayNameOfSVDHits; }

    /// Name of the store array of the pxd hits.
    const std::string& getStoreArrayNameOfPXDHits() const { return m_storeArrayNameOfPXDHits; }

    /// Name of the store array of the bklm hits.
    const std::string& getStoreArrayNameOfBKLMHits() const { return m_storeArrayNameOfBKLMHits; }

    /// Name of the store array of the eklm hits.
    const std::string& getStoreArrayNameOfEKLMHits() const { return m_storeArrayNameOfEKLMHits; }

    /// Name of the store array of the reco hit informations.
    const std::string& getStoreArrayNameOfRecoHitInformation() const { return m_storeArrayNameOfRecoHitInformation; }

    /**
     * Call a function on all hits of the given type in the store array, that are related to this track.
     * @param storeArrayNameOfHits The store array the hits should come from.
     * @param mapFunction Call this function for every hit (with its reco hit information)
     * @param pickFunction Use only those hits where the function returns true.
     */
    template<class HitType>
    void mapOnHits(const std::string& storeArrayNameOfHits,
                   std::function<void(RecoHitInformation&, HitType*)> const&   mapFunction,
                   std::function<bool(const RecoHitInformation&, const HitType*)> const& pickFunction)
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
     * @param storeArrayNameOfHits The store array the hits should come from.
     * @param mapFunction Call this function for every hit (with its reco hit information)
     * @param pickFunction Use only those hits where the function returns true.
     */
    template<class HitType>
    void mapOnHits(const std::string& storeArrayNameOfHits,
                   std::function<void(const RecoHitInformation&, const HitType*)> const& mapFunction,
                   std::function<bool(const RecoHitInformation&, const HitType*)> const& pickFunction) const
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
     * @param storeArrayNameOfHits The store array the hits should come from.
     * @param mapFunction Call this function for every hit (with its reco hit information)
     */
    template<class HitType>
    void mapOnHits(const std::string& storeArrayNameOfHits,
                   std::function<void(RecoHitInformation&, HitType*)> const& mapFunction)
    {
      mapOnHits<HitType>(storeArrayNameOfHits, mapFunction, [](const RecoHitInformation&, const HitType*) -> bool { return true; });
    }

    /**
     * Call a function on all hits of the given type in the store array, that are related to this track. Const version.
     * @param storeArrayNameOfHits The store array the hits should come from.
     * @param mapFunction Call this function for every hit (with its reco hit information)
     */
    template<class HitType>
    void mapOnHits(const std::string& storeArrayNameOfHits,
                   std::function<void(const RecoHitInformation&, const HitType*)> const&   mapFunction) const
    {
      mapOnHits<HitType>(storeArrayNameOfHits, mapFunction, [](const RecoHitInformation&, const HitType*) -> bool { return true; });
    }

    // Matching status
    /// Return the matching status set by the TrackMatcher module
    MatchingStatus getMatchingStatus() const
    {
      return m_matchingStatus;
    }

    /// Set the matching status (used by the TrackMatcher module)
    void setMatchingStatus(MatchingStatus matchingStatus)
    {
      m_matchingStatus = matchingStatus;
    }

    /// Get the quality index attached to this RecoTrack given by one of the reconstruction algorithms. 0 means likely fake.
    float getQualityIndicator() const
    {
      return m_qualityIndicator;
    }

    /// Set the quality index attached to this RecoTrack. 0 means likely fake.
    void setQualityIndicator(float qualityIndicator)
    {
      m_qualityIndicator = qualityIndicator;
    }

    /**
     * Delete all fitted information for all representations.
     *
     * This function is needed, when you want to start the fitting "from scratch".
     * After this function, a fit will recreate all measurements and fitted information.
     * Please be aware that any pointers will be invalid after that!
     */
    void deleteFittedInformation();

    /**
     * Delete all fitted information for the given representations.
     *
     * This function is needed, when you want to start the fitting "from scratch".
     * After this function, a fit will recreate all measurements and fitted information.
     * Please be aware that any pointers will be invalid after that!
     */
    void deleteFittedInformationForRepresentation(const genfit::AbsTrackRep* rep);

    /// Get useful information on EventDisplay
    virtual std::string getInfoHTML() const;

  private:
    /// Internal storage for the genfit track.
    genfit::Track m_genfitTrack;
    /// Storage for the charge. All other helix parameters are saved in the genfit::Track.
    short int m_charge = 1;
    /// Store array name of added CDC hits.
    std::string m_storeArrayNameOfCDCHits = "";
    /// Store array name of added SVD hits.
    std::string m_storeArrayNameOfSVDHits = "";
    /// Store array name of added PXD hits.
    std::string m_storeArrayNameOfPXDHits = "";
    /// Store array name of added BKLM hits.
    std::string m_storeArrayNameOfBKLMHits = "";
    /// Store array name of added EKLM hits.
    std::string m_storeArrayNameOfEKLMHits = "";
    /// Store array of added RecoHitInformation.
    std::string m_storeArrayNameOfRecoHitInformation = "";
    /// Bool is hits were added to track after fitting and the measurements should be recalculated.
    /// will be true after ROOT deserialization, which means the measurements will be recreated
    bool m_dirtyFlag = true;
    /// Flag used in the MCRecoTracksMatcherModule
    MatchingStatus m_matchingStatus = MatchingStatus::c_undefined;
    /// Quality index for classification of fake vs. MC-matched Tracks.
    float m_qualityIndicator = NAN;

    /**
     * Add a generic hit with the given parameters for the reco hit information.
     * @param hit a generic hit.
     * @param params for the constructor of the reco hit information.
     * @return true if the hit was new.
     */
    template<class HitType, class ...Args>
    bool addHit(const HitType* hit, Args&& ... params)
    {
      if (hasHit(hit)) {
        return false;
      }

      StoreArray<RecoHitInformation> recoHitInformations(m_storeArrayNameOfRecoHitInformation);
      RecoHitInformation* newRecoHitInformation = recoHitInformations.appendNew(hit, params...);

      addHitWithHitInformation(hit, newRecoHitInformation);

      return true;
    }

    /**
     * Add the needed relations for adding a generic hit with the given hit information.
     * @param hit The hit to add
     * @param recoHitInformation The reco hit information of the hit.
     */
    template <class HitType>
    void addHitWithHitInformation(const HitType* hit, RecoHitInformation* recoHitInformation)
    {
      hit->addRelationTo(this);
      addRelationTo(recoHitInformation);

      setDirtyFlag();
    }

    /// Returns the reco hit information for a given hit or throws an exception if the hit is not related to the track.
    template <class HitType>
    RecoHitInformation* getRecoHitInformationSafely(HitType* hit) const
    {
      RecoHitInformation* recoHitInformation = getRecoHitInformation(hit);
      if (recoHitInformation == nullptr) {
        B2FATAL("Queried hit is not in the reco track! Did you prune it?");
      } else {
        return recoHitInformation;
      }
    }

    /**
     * Get the number of hits for the given hit type in the store array that are related to this track.
     * @param storeArrayNameOfHits The StoreArray to look for.
     */
    template <class HitType>
    unsigned int getNumberOfHitsOfGivenType(const std::string& storeArrayNameOfHits) const
    {
      return getRelationsFrom<HitType>(storeArrayNameOfHits).size();
    }

    /**
     * Return a sorted list of hits of the given type in the store array that are related to this track.
     * @param storeArrayNameOfHits The StoreArray to look for.
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

    /**
     * Return an unsorted list of hits of the given type in the store array that are related to this track.
     * @param storeArrayNameOfHits The StoreArray to look for.
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

    /// Helper: Check the dirty flag and produce a warning, whenever a fit result is accessed.
    void checkDirtyFlag() const
    {
      if (m_dirtyFlag) {
        B2DEBUG(100, "Dirty flag is set. The result may not be in sync with the latest changes. Refit the track to be sure.");
      }
    }

    /** Making this class a ROOT class.*/
    ClassDef(RecoTrack, 8);
  };

  /**
   * This class allows access to the genfit::Track of the RecoTrack.
   *
   * This class allows direct access to the most holy part of the RecoTrack. The design of the RecoTrack is such, that this should not be required.
   * However, some interfaces require a genfit::Track, e.g. the genfit rave interface, and the access to the genfit::Track member is required.
   * This should only be used when no other solution works.
   */
  class RecoTrackGenfitAccess {
  public:
    /**
     * Give access to the RecoTrack's genfit::Track.
     *
     * @param recoTrack  Track to unpack
     * @return genfit::Track of the RecoTrack.
     */
    static genfit::Track& getGenfitTrack(RecoTrack& recoTrack);

    /**
     * Checks if a TrackRap for the PDG id of the RecoTrack (and its charge conjugate) does
     * already exit and returns it if available. If no TrackRep is available, a new RKTrackRep
     * is added to the genfit::Track. This ensures that a TrackRep with the same PDG id
     * (and its charge conjugate) is not available two times in the genfit::Track.
     *
     * By convention, only one TrackRep for one particle type can exist
     * inside of a RecoTrack, no matter the charge. So there can only be a electron or positron TrackRep,
     * but not both.
     *
     * @param recoTrack Track to add TrackRep to
     * @param PDG code of the hypothesis which is negative or positive, depending on
     * the charge of the hypothesis particle.
     */
    static genfit::AbsTrackRep* createOrReturnRKTrackRep(RecoTrack& recoTrack, int PDGcode);
  };

}

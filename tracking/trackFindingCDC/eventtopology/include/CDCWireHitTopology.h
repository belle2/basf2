/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <cdc/dataobjects/CDCHit.h>

#include <tracking/trackFindingCDC/rootification/SwitchableRootificationBase.h>
#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/topology/CDCWireLayer.h>
#include <tracking/trackFindingCDC/topology/CDCWireSuperLayer.h>


#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRLWireHit.h>

#include <tracking/trackFindingCDC/algorithms/SortableVector.h>

#include <cdc/dataobjects/TDCCountTranslatorBase.h>
#include <framework/dataobjects/EventMetaData.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class representating the wire hit arrangement in the current event in the whole of the central drift chamber.
    /** This class stores the hits in the CDC that have been unpacked from the raw CDCHits and geometrical information
     *  of the wire position an initial estimate of the drift length is attached.
     *  Also a pair of oriented wire hits (with an assoziate right left passage hypotheses) is stored in here.
     *  In this way track finders do not need to (re)create hits multiple times but simply refer to them.
     *
     *  For faster lookup the hits are sorted by their corresponding wire id and their reference drift radius
     *  (and their right left passage hypothese in case of
     *  So they can be retieved by their respective (super) layer as a range.
     */
    class CDCWireHitTopology : public SwitchableRootificationBase {

    public:
      /// A Range of const CDCWireHits - usable with range based for
      typedef SortableVector<Belle2::TrackFindingCDC::CDCWireHit>::const_range CDCWireHitRange;

      /// A Range of const CDCLRWireHits - usable with range based for
      typedef SortableVector<Belle2::TrackFindingCDC::CDCRLWireHit>::const_range CDCRLWireHitRange;

      /// Getter of the singletone instance
      static CDCWireHitTopology& getInstance();

    public:
      /// Default constructor for ROOT compatibility.
      CDCWireHitTopology();

      /// Empty deconstructor
      ~CDCWireHitTopology();

    public:
      /// Initialize the wire hit topology at the begin of the run. Requires the StoreArray of CDCHits.
      void initialize();

      /// Fill the topology from the raw cdc hits.
      size_t event();

      /// Use all cdc hits
      size_t useAll();

      /// Blocks the cdc hits given as indices to the store array. Returns the number of blocked hits
      size_t dontUse(const std::vector<int>& iHits);

      /// Use all but the cdc hits given as indices to the store array.
      size_t useAllBut(const std::vector<int>& iHits);

      /// Only use the cdc hits given as indices to the store array.
      size_t useOnly(const std::vector<int>& iHits);

      /// Only use the cdc hits that have a relation from the StoreArray given by full name.
      size_t dontUseRelatedFrom(const std::string& storeArrayName)
      {
        std::vector<int> iHits = getIHitsRelatedFrom(storeArrayName);
        return dontUse(iHits);
      }

      /// Only use the cdc hits that have a relation from the StoreArray given by full name.
      size_t useOnlyRelatedFrom(const std::string& storeArrayName)
      {
        std::vector<int> iHits = getIHitsRelatedFrom(storeArrayName);
        return useOnly(iHits);
      }

      /// Use all but the cdc hits that have a relation from the StoreArray given by full name.
      size_t useAllButRelatedFrom(const std::string& storeArrayName)
      {
        std::vector<int> iHits = getIHitsRelatedFrom(storeArrayName);
        return useAllBut(iHits);
      }

    private:
      /// Returns all indices of cdc hits that have a relation from the StoreArray given by its full name.
      std::vector<int> getIHitsRelatedFrom(const std::string& storeArrayName) const;

    private:
      /**
       * Fill the topology from the raw cdc hits
       * Note this methode should only called once per event,
       * since it is likely that pointers to the translated hits are keep through out the reconstruction procedure.
       *
       * @param  cdcHitsStoreArrayName  Name of the store array the raw cdc hits shall be used for tracking.
       */
      size_t fill(const std::string& cdcHitsStoreArrayName);

      /// Clear content of the topology after the event is processed.
      void clear();

    public:
      /// Getter for the oriented wire hit with the opposite orientation.
      const Belle2::TrackFindingCDC::CDCRLWireHit* getReverseOf(const Belle2::TrackFindingCDC::CDCRLWireHit& rlWireHit) const;

      /// Getter for the wire hit that is based on the given CDCHit.
      const Belle2::TrackFindingCDC::CDCWireHit* getWireHit(const Belle2::CDCHit* ptrHit) const;

      /// Getter for the two oriented wire hits that are based on the given wire hit
      std::pair<const Belle2::TrackFindingCDC::CDCRLWireHit*, const Belle2::TrackFindingCDC::CDCRLWireHit*> getRLWireHitPair(
        const Belle2::TrackFindingCDC::CDCWireHit& wireHit) const;

      /// Getter for the oriented wire hit that is based on the given wire hit with a specific right left passage hypotheses.
      const Belle2::TrackFindingCDC::CDCRLWireHit* getRLWireHit(const Belle2::TrackFindingCDC::CDCWireHit& wireHit,
                                                                const RightLeftInfo& rlInfo) const;

      /// Getter for the oriented wire hit that is based on the given CDCHit with a specific right left passafe hypotheses.
      const Belle2::TrackFindingCDC::CDCRLWireHit* getRLWireHit(const Belle2::CDCHit* ptrHit, const RightLeftInfo& rlInfo) const;

      /// Getter for a coaligned subrange of wire hits.
      template<class Coaligned>
      CDCWireHitRange getWireHits(const Coaligned& coaligned) const { return m_wireHits.equal_range(coaligned); }

      /// Getter for a coaligned subrange of oriented wire hits
      template<class Coaligned>
      CDCRLWireHitRange getRLWireHits(const Coaligned& coaligned) const { return m_rlWireHits.equal_range(coaligned); }

      /// Getter for the wire hits
      const Belle2::TrackFindingCDC::SortableVector<Belle2::TrackFindingCDC::CDCWireHit>& getWireHits() const
      { return m_wireHits; }

      /// Getter for the wire hits
      const Belle2::TrackFindingCDC::SortableVector<Belle2::TrackFindingCDC::CDCRLWireHit>& getRLWireHits() const
      { return m_rlWireHits; }

      /// Getter for the tdc count translator that was used at the beginning of the event.
      CDC::TDCCountTranslatorBase& getTDCCountTranslator()
      { return *m_initialTDCCountTranslator; }

    private:

      /// Memory for the event, run and experminent number corresponding to the currently stored data.
      EventMetaData m_eventMetaData;

      SortableVector<CDCWireHit> m_wireHits; ///< Memory for the wire hits to be stored
      SortableVector<CDCRLWireHit> m_rlWireHits; ///< Memory for the oriented wire hits to be stored

      bool m_useSimpleTDCCountTranslator; ///< Switch whether the simple or the realistic tdc count translator shall be used.

      CDC::TDCCountTranslatorBase*
      m_initialTDCCountTranslator; ///< Reference of the tdc count translator used at the beginning of this event.

    private:

    }; //class


  } // namespace TrackFindingCDC
} // namespace Belle2

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


#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/topology/CDCWireLayer.h>
#include <tracking/trackFindingCDC/topology/CDCWireSuperLayer.h>


#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

#include <tracking/trackFindingCDC/utilities/Range.h>

#include <cdc/dataobjects/TDCCountTranslatorBase.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationsObject.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCSimpleSimulation;

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
    class CDCWireHitTopology : public TObject {

    public:
      /// A Range of const CDCWireHits - usable with range based for
      typedef Range<std::vector<Belle2::TrackFindingCDC::CDCWireHit>::const_iterator> CDCWireHitRange;

      /// A Range of const CDCLRWireHits - usable with range based for
      typedef Range<std::vector<Belle2::TrackFindingCDC::CDCRLWireHit>::const_iterator> CDCRLWireHitRange;

      /// Getter of the singletone instance
      static CDCWireHitTopology& getInstance();

      /// Create the StoreObject for the WireHitTopology.
      static void initialize();

      /// Default constructor for ROOT compatibility.
      CDCWireHitTopology();

      /// Deconstructor
      ~CDCWireHitTopology();

      /** Fill the topology with preconstructed wire hits*/
      void fill(const std::vector<CDCWireHit>& wireHits);

      /// Clear content of the topology after the event is processed.
      void clear();

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

      /// Getter for the oriented wire hit with the opposite orientation.
      const Belle2::TrackFindingCDC::CDCRLWireHit*
      getReverseOf(const Belle2::TrackFindingCDC::CDCRLWireHit& rlWireHit) const;

      /// Getter for the wire hit that is based on the given CDCHit.
      const Belle2::TrackFindingCDC::CDCWireHit*
      getWireHit(const Belle2::CDCHit* ptrHit) const;

      /// Getter for the two oriented wire hits that are based on the given wire hit
      std::pair<const Belle2::TrackFindingCDC::CDCRLWireHit*,
          const Belle2::TrackFindingCDC::CDCRLWireHit*>
          getRLWireHitPair(const Belle2::TrackFindingCDC::CDCWireHit& wireHit) const;

      /// Getter for the oriented wire hit that is based on the given wire hit with a specific right left passage hypotheses.
      const Belle2::TrackFindingCDC::CDCRLWireHit*
      getRLWireHit(const Belle2::TrackFindingCDC::CDCWireHit& wireHit,
                   const ERightLeft rlInfo) const;

      /// Getter for the oriented wire hit that is based on the given CDCHit with a specific right left passage hypotheses.
      const Belle2::TrackFindingCDC::CDCRLWireHit*
      getRLWireHit(const Belle2::CDCHit* ptrHit,
                   const ERightLeft rlInfo) const;

      /// Getter for a coaligned subrange of wire hits.
      template<class ACoaligned>
      CDCWireHitRange getWireHits(const ACoaligned& coaligned) const
      { return std::equal_range(getWireHits().begin(), getWireHits().end(), coaligned); }

      /// Getter for a coaligned subrange of oriented wire hits
      template<class ACoaligned>
      CDCRLWireHitRange getRLWireHits(const ACoaligned& coaligned) const
      { return std::equal_range(getRLWireHits().begin(), getRLWireHits().end(), coaligned); }

      /// Constant getter for the wire hits
      const std::vector<Belle2::TrackFindingCDC::CDCWireHit>& getWireHits() const
      { return m_wireHits; }

      /// Getter for the wire hits
      std::vector<Belle2::TrackFindingCDC::CDCWireHit>& getWireHits()
      { return m_wireHits; }

      /// Getter for the wire hits
      const std::vector<Belle2::TrackFindingCDC::CDCRLWireHit>& getRLWireHits() const
      { return m_rlWireHits; }

      /// Getter for the tdc count translator that was used at the beginning of the event.
      CDC::TDCCountTranslatorBase& getTDCCountTranslator()
      { return *m_initialTDCCountTranslator; }

    private:
      /// Memory for the wire hits to be stored
      std::vector<CDCWireHit> m_wireHits; //!

      /// Memory for the oriented wire hits to be stored
      std::vector<CDCRLWireHit> m_rlWireHits; //!

      /// Reference of the tdc count translator used at the beginning of this event.
      CDC::TDCCountTranslatorBase* m_initialTDCCountTranslator; //!

      /// Allow the simple simulation to manipulate the objects directly
      friend CDCSimpleSimulation;

      /// Returns all indices of cdc hits that have a relation from the StoreArray given by its full name.
      std::vector<int> getIHitsRelatedFrom(const std::string& storeArrayName) const;


      ClassDef(CDCWireHitTopology, 1);
    }; //class


  } // namespace TrackFindingCDC
} // namespace Belle2

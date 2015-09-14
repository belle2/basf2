/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/trackFinderOutputCombining/MatchingInformation.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoHit2D;
    class CDCRecoSegment2D;

    class CDCRecoHit3D;
    class CDCTrack;

    /** Abstract base class for a list of objects which could be filled (more or less a vector) */
    template <class ItemType, class ListType>
    class LookUpBase {
    public:
      /** Should be overloaded. Fill the vector with the given elements */
      virtual void fillWith(std::vector<ItemType>& items) = 0;
      virtual ~LookUpBase() { }

      /// Constant iterator type of this container
      typedef typename std::vector<ListType>::const_iterator const_iterator;
      /// Constant iterator type of this container
      typedef typename std::vector<ListType>::iterator iterator;

      /** STL: begin */
      const_iterator begin() const
      {
        return m_lookup.begin();
      }

      /** STL: begin */
      iterator begin()
      {
        return m_lookup.begin();
      }

      /** STL: end */
      const_iterator end() const
      {
        return m_lookup.end();
      }

      /** STL: end */
      iterator end()
      {
        return m_lookup.end();
      }

    protected:
      std::vector<ListType> m_lookup; /**< The internal store for the elements list */
    };

    /** We use this class for storing our segment lists - one for each superlayer
     * This lookup serves (as an addon) also as lookup for the relation cdchit -> segment
     * When filling the lookup we only process not already taken segments.
     * */
    class SegmentLookUp : public LookUpBase<CDCRecoSegment2D, std::vector<SegmentInformation*>> {
    public:
      /** Create the lists. Do not forget to call clear before the next event. */
      void fillWith(std::vector<CDCRecoSegment2D>& segments) override;

      /** Clear all pointer vectors */
      void clear();

      /** Return the segment that has this hit */
      SegmentInformation* findSegmentForHit(const CDCRecoHit3D& recoHit);

    private:
      /// The added hit -> segment lookup
      std::map<const CDCHit*, SegmentInformation*> m_hitSegmentLookUp;
    };

    /** And we use this class for storing our Track Information
     * This lookup serves (as an addon) also as lookup for the relation cdchit -> track
     * */
    class TrackLookUp : public LookUpBase<CDCTrack, TrackInformation*> {
    public:
      /** Create the lists. Do not forget to call clear before the next event. */
      void fillWith(std::vector<CDCTrack>& tracks) override;

      /** Clear all pointer vectors */
      void clear();

      /** Return the track that has this hit */
      TrackInformation* findTrackForHit(const CDCRecoHit2D& recoHit);

    private:
      /// The added hit -> track lookup
      std::map<const CDCHit*, TrackInformation*> m_hitTrackLookUp;
    };
  }
}

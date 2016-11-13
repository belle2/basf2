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

#include <vector>
#include <map>

namespace Belle2 {
  /**
   * The classes in this file serve as base structure for the segment track combiner.
   * They offer quick lookups of the relation between hits and segments/tracks
   * and stores the segment/tracks together with some flag information
   * needed in the segment track combiner.
   *
   * For information on the data structure of the elements, see MatchingInformation.h.
   */
  class CDCHit;

  namespace TrackFindingCDC {
    class CDCRecoHit2D;
    class CDCRecoHit3D;
    class CDCRecoSegment2D;
    class CDCTrack;
    class TrackInformation;
    class SegmentInformation;

    /**
     * Abstract base class for a list of objects.
     * This base class offers the interface for later look ups.
     * It passes the vector functions to the outside as far as they are needed.
     */
    template<class ItemType, class ListType>
    class LookUpBase {
    public:
      /// Constant iterator type of this container.
      using const_iterator = typename std::vector<ListType>::const_iterator;

      /// Constant iterator type of this container.
      using iterator = typename std::vector<ListType>::iterator;

      /// Make destructor of interface virtual
      virtual ~LookUpBase() = default;

      /** Should be overloaded. Fill the vector with the given elements. */
      virtual void fillWith(std::vector<ItemType>& items) = 0;

      /** STL: begin. */
      const_iterator begin() const
      {
        return m_lookup.begin();
      }

      /** STL: begin. */
      iterator begin()
      {
        return m_lookup.begin();
      }

      /** STL: end. */
      const_iterator end() const
      {
        return m_lookup.end();
      }

      /** STL: end. */
      iterator end()
      {
        return m_lookup.end();
      }

      /** Clear all pointer vectors. */
      virtual void clear() = 0;

    protected:
      /// The internal store for the elements list.
      std::vector<ListType> m_lookup;
    };

    /** We use this class for storing our segment lists - one for each superlayer.
     * This lookup serves (as an addon) also as lookup for the relation cdchit -> segment
     * When filling the lookup we only process not already taken segments.
     * */
    class SegmentLookUp : public LookUpBase<CDCRecoSegment2D, std::vector<SegmentInformation*>> {
    public:
      /** Create the lists. Do not forget to call clear before the next event. */
      void fillWith(std::vector<CDCRecoSegment2D>& segments) override;

      /** Clear all pointer vectors. */
      void clear() override;

      /** Return the segment that has this hit. */
      SegmentInformation* findSegmentForHit(const CDCRecoHit3D& recoHit);

    private:
      /// The added hit -> segment lookup.
      std::map<const CDCHit*, SegmentInformation*> m_hitSegmentLookUp;
    };

    /** And we use this class for storing our Track Information.
     * This lookup serves (as an addon) also as lookup for the relation cdchit -> track.
     * */
    class TrackLookUp : public LookUpBase<CDCTrack, TrackInformation*> {
    public:
      /** Create the lists. Do not forget to call clear before the next event. */
      void fillWith(std::vector<CDCTrack>& tracks) override;

      /** Clear all pointer vectors. */
      void clear() override;

      /** Return the track that has this hit. */
      TrackInformation* findTrackForHit(const CDCRecoHit2D& recoHit);

    private:
      /// The added hit -> track lookup.
      std::map<const CDCHit*, TrackInformation*> m_hitTrackLookUp;
    };
  }
}

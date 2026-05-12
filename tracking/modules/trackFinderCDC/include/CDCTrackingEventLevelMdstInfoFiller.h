/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/eventdata/utils/ClassMnemomics.h>

#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>
#include <tracking/trackingUtilities/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackingUtilities/findlets/base/FindletModule.h>
#include <tracking/trackingUtilities/findlets/base/Findlet.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     *  Findlet for storing in mdsts some hit-based information per event.
     */
    class CDCTrackingEventLevelMdstInfoFillerFromHitsFindlet : public TrackingUtilities::Findlet<const TrackingUtilities::CDCWireHit> {
    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<const TrackingUtilities::CDCWireHit>;

    public:
      /// Getter for the module description
      std::string getDescription() final;

      /// Called at the beginning of the event processing
      void initialize() final;

    public:
      /// Main method to make the heavy lifting
      void apply(const std::vector<TrackingUtilities::CDCWireHit>& inputWireHits) final;

    private:
      /** Access to the EventLevelTrackingInfo object in the datastore. */
      StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo;
    };

    /**
     *  Findlet for storing in mdsts some segment-based information per event.
     */
    class CDCTrackingEventLevelMdstInfoFillerFromSegmentsFindlet : public
      TrackingUtilities::Findlet<const TrackingUtilities::CDCSegment2D> {
    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<const TrackingUtilities::CDCSegment2D>;

    public:
      /// Getter for the module description
      std::string getDescription() final;

      /// Called at the beginning of the event processing
      void initialize() final;

    public:
      /// Main method to make the heavy lifting
      void apply(const std::vector<TrackingUtilities::CDCSegment2D>& inputSegment2Ds) final;

    private:
      /** Access to the EventLevelTrackingInfo object in the datastore. */
      StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo;
    };

    /**
     *  Module interface to the CDCTrackingEventLevelMdstInfoFillerFromHitsFindlet
     */
    class CDCTrackingEventLevelMdstInfoFillerFromHitsModule : public
      TrackingUtilities::FindletModule<CDCTrackingEventLevelMdstInfoFillerFromHitsFindlet> {
    public:
      /// Constructor setting up the default store array names
      CDCTrackingEventLevelMdstInfoFillerFromHitsModule()
        : TrackingUtilities::FindletModule<CDCTrackingEventLevelMdstInfoFillerFromHitsFindlet>({"CDCWireHitVector"})
      {
      }
    };

    /**
     *  Module interface to the CDCTrackingEventLevelMdstInfoFillerFromSegmentsFindlet
     */
    class CDCTrackingEventLevelMdstInfoFillerFromSegmentsModule : public
      TrackingUtilities::FindletModule<CDCTrackingEventLevelMdstInfoFillerFromSegmentsFindlet> {
    public:
      /// Constructor setting up the default store array names
      CDCTrackingEventLevelMdstInfoFillerFromSegmentsModule()
        : TrackingUtilities::FindletModule<CDCTrackingEventLevelMdstInfoFillerFromSegmentsFindlet>({"CDCSegment2DVector"})
      {
      }
    };
  }
}

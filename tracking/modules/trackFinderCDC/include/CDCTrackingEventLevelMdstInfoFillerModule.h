/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/utils/ClassMnemomics.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/findlets/base/FindletModule.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     *  Findlet description.
     *  This may go to another header file.
     */
    class CDCTrackingEventLevelMdstInfoFillerFindlet : public Findlet<const CDCWireHit, const CDCSegment2D> {
    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHit, const CDCSegment2D>;

    public:
      /// Getter for the module description
      std::string getDescription() final;

      /// Signal the begining of the event processing
      void initialize() final;

    public:
      /// Main method to make the heavy lifting
      void apply(const std::vector<CDCWireHit>& inputWireHits, const std::vector<CDCSegment2D>& inputWireHitSegments) final;

    private:
      /** Acccess to the CDCTrackingEventLevelTrackingInfo object in the datastore. */
      StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo;
    };

    /**
     *  Module interface to the CDCTrackingEventLevelMdstInfoFillerFindlet
     */
    class CDCTrackingEventLevelMdstInfoFillerModule : public FindletModule<CDCTrackingEventLevelMdstInfoFillerFindlet> {
    public:
      /// Constructor setting up the default store array names
      CDCTrackingEventLevelMdstInfoFillerModule()
        : FindletModule<CDCTrackingEventLevelMdstInfoFillerFindlet>( {"CDCWireHitVector", "CDCSegment2DVector"})
      {
      }
    };
  }
}

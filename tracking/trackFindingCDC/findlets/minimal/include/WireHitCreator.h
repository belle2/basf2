/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/CDCHit.h>

#include <framework/datastore/StoreArray.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /* Combines the geometrical information and the raw hit information into wire hits, which can be used from all modules after that. */
    class WireHitCreator : public Findlet<CDCWireHit> {
    private:
      /// Type of the base class
      typedef Findlet<CDCWireHit> Super;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Combines the geometrical information and the raw hit information into wire hits, "
               "which can be used from all modules after that";
      }

      /// Expose the parameters to a module
      virtual void exposeParameters(ModuleParamList*) override final
      {
        // No parameters
      }

      /// Signals the start of the event processing
      virtual void initialize() override final
      {
        StoreArray<CDCHit> hits;
        hits.isRequired();

        // Preload geometry during initialization
        // Marked as unused intentionally to avoid a compile warning
        CDC::CDCGeometryPar& cdcGeo __attribute__((unused)) = CDC::CDCGeometryPar::Instance();
        CDCWireTopology& wireTopology  __attribute__((unused)) = CDCWireTopology::getInstance();

        m_tdcCountTranslator.reset(new CDC::RealisticTDCCountTranslator);
        Super::initialize();
      }

      /// Main algorithm creating the wire hits
      virtual void apply(std::vector<CDCWireHit>& outputWireHits) override final
      {
        StoreArray<CDCHit> hits;
        hits.isRequired();

        // Create the wirehits into a vector
        Index useNHits = hits.getEntries();
        outputWireHits.reserve(useNHits);

        for (CDCHit& hit : hits) {
          outputWireHits.push_back(CDCWireHit(&hit, m_tdcCountTranslator.get()));
        }

        // Some safety checks from funky times - did not trigger for ages.
        for (Index iHit = 0; iHit < useNHits; ++iHit) {
          CDCHit* ptrHit = hits[iHit];
          CDCHit& hit = *ptrHit;
          const WireID wireID(hit.getID());
          const CDCWireHit& wireHit = outputWireHits[iHit];

          if (iHit != hit.getArrayIndex()) {
            B2ERROR("CDCHit.getArrayIndex() produced wrong result. Expected : " << iHit << " Actual : " << hit.getArrayIndex());
          }

          if (wireID.getEWire() != hit.getID()) {
            B2ERROR("WireID.getEWire() differs from CDCHit.getID()");
          }

          if (hit.getID() != wireHit.getWire().getEWire()) {
            B2ERROR("CDCHit.getID() differs from CDCWireHit.getWire().getEWire()");
          }
          if (hit.getArrayIndex() != wireHit.getStoreIHit()) {
            B2ERROR("CDCHit.getArrayIndex() differs from CDCWireHit.getStoreIHit");
          }
        }

        B2DEBUG(100, "  Created number of CDCWireHits == " << useNHits);
        B2DEBUG(100, "  Number of usable CDCWireHits == " << useNHits);

        if (useNHits == 0) {
          B2WARNING("Event with no hits");
        }

        std::sort(outputWireHits.begin(), outputWireHits.end());
      }

    private:
      /// TDC Count translator to be used to calculate the initial dirft length estiamtes
      std::unique_ptr<CDC::RealisticTDCCountTranslator> m_tdcCountTranslator{nullptr};
      // Note we can only create it on initialisation because the gearbox has to be connected.

    }; // end class WireHitCreator

  } // end namespace TrackFindingCDC
} // end namespace Belle2

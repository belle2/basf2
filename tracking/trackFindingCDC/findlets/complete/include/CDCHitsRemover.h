/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/SelectSubset.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <cdc/dataobjects/CDCHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     *  Findlet to remove background hits from CDCHits list. Useful for background studies.
     */
    class CDCHitsRemover : public Findlet<const CDCWireHit> {
    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHit>;
    public:
      /**
       * Constructor
       */
      CDCHitsRemover() = default;
      /**
       * Initialize.
       * This method is called at the beginning of data processing.
       */
      void initialize() final;

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)  final;

      /**
       * Event processor.
       */
      void apply(const std::vector<CDCWireHit>& wireHits) override;
    private:
      StoreArray<CDCHit>     m_cdcHits;     /* CDC hits */
      SelectSubset<CDCHit>   m_cdc_selector; /* Selector of CDC hits */
    };
  };
}

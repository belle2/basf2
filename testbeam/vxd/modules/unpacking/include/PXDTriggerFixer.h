#ifndef PXDTRIGGERFIXERMODULE_H_
#define PXDTRIGGERFIXERMODULE_H_

#include <framework/core/Module.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <rawdata/dataobjects/RawPXD.h>

#include <framework/core/MRUCache.h>

namespace Belle2 {

  namespace PXD {
    /// Module to match PXD and HLT/Meta trigger number which differ (including an offset)
    class PXDTriggerFixerModule: public Module {
    private:
      /// MRU cache size
      enum {MAX_EVENTSHIFT = 1000};
      /// MRU cache with last couple of events
      MRUCache<int, TClonesArray> m_previous_events{MAX_EVENTSHIFT};
      /// The store array with RawPXDs to match
      StoreArray<RawPXD> m_storeRaw;
      /// The optional trigger offset
      int m_offset;
      bool m_refs_HLT; /// Use HLT Trigger number as reference with PXD packet
      bool m_refs_DHC ; /// Use DHC Trigger number as reference with PXD packet
      bool m_refs_DHE; /// Use (first) DHE Trigger number as reference with PXD packet
      bool m_wants_Meta; /// Look for EvtMeta Trigger number
      bool m_wants_HLT; /// Look for HLT Trigger number
      bool m_wants_DHC; /// Look for DHC Trigger number

      unsigned int m_fixed{0}, m_notfixed{0}, m_notneeded{0};

    public:
      PXDTriggerFixerModule();
      void initialize(void);
      void terminate(void);
      void event(void);
      /// Get the DHH and HLT trigger number from RawPXD
      bool getTrigNr(RawPXD& px, unsigned int& innerDHE, unsigned int& innerDHC, unsigned int& outerHLT);
      /// Unpack DHE(C) frame in dataptr
      bool unpack_dhc_frame(void* data, unsigned int& innerDHE, unsigned int& innerDHC, unsigned int& outerHLT);
    };
  }
}
#endif

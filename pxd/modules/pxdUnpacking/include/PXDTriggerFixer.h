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

#define MAX_EVENTSHIFT int(100)

    class PXDTriggerFixerModule: public Module {
    private:
      MRUCache<int, TClonesArray> m_previous_events{MAX_EVENTSHIFT};
      StoreArray<RawPXD> m_storeRaw;

    public:
      void initialize(void);
      void event(void);
      bool getTrigNr(RawPXD& px, unsigned int& innerDHH, unsigned int& outerHLT);
      bool unpack_dhc_frame(void* data, unsigned int& innerDHH, unsigned int& outerHLT);
    };
  }
}
#endif
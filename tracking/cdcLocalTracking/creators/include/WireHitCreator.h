/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef WIREHITCREATOR_H_
#define WIREHITCREATOR_H_

#include <cdc/dataobjects/CDCHit.h>

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  namespace CDCLocalTracking {
    /// Class providing the initial construction of wirehits
    class WireHitCreator {

    public:

      /** Constructor. */
      WireHitCreator();

      /** Destructor.*/
      ~WireHitCreator();

      typedef Belle2::CDCLocalTracking::CDCWireHitCollection CDCWireHitCollection;

      void create(StoreArray <CDCHit>& storedCDCHits, CDCWireHitCollection& wirehits) const;

    private:


    }; // end class WireHitCreator


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //WIREHITCREATOR_H_

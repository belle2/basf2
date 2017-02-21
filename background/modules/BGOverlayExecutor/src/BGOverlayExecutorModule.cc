/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BGOverlayExecutor/BGOverlayExecutorModule.h>



// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// detector Digits, Clusters or waveforms
#include <pxd/dataobjects/PXDDigit.h>
#include <svd/dataobjects/SVDDigit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <arich/dataobjects/ARICHDigit.h>
#include <ecl/dataobjects/ECLDigit.h> // or waveform, not yet clear
#include <bklm/dataobjects/BKLMDigit.h>
#include <eklm/dataobjects/EKLMDigit.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(BGOverlayExecutor)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  BGOverlayExecutorModule::BGOverlayExecutorModule() : Module()

  {
    // module description
    setDescription("Overlay of measured background with simulated data");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters

  }

  BGOverlayExecutorModule::~BGOverlayExecutorModule()
  {
  }

  void BGOverlayExecutorModule::initialize()
  {

    // registration in datastore (all as optional input - see template function)
    registerDigits<PXDDigit>();
    registerDigits<SVDDigit>();
    registerDigits<CDCHit>();
    registerDigits<TOPDigit>();
    registerDigits<ARICHDigit>();
    registerDigits<ECLDigit>(); // not yet clear which one
    registerDigits<BKLMDigit>();
    registerDigits<EKLMDigit>();

  }

  void BGOverlayExecutorModule::beginRun()
  {
  }

  void BGOverlayExecutorModule::event()
  {
    /* note: dataobject must inherit from DigitBase */

    addBGDigits<PXDDigit>();
    addBGDigits<SVDDigit>();
    addBGDigits<CDCHit>();
    addBGDigits<TOPDigit>();
    addBGDigits<ARICHDigit>();
    //    addBGDigits<ECLDigit>(); // not yet clear which one
    addBGDigits<BKLMDigit>();
    addBGDigits<EKLMDigit>();

  }


  void BGOverlayExecutorModule::endRun()
  {
  }

  void BGOverlayExecutorModule::terminate()
  {
  }


} // end Belle2 namespace


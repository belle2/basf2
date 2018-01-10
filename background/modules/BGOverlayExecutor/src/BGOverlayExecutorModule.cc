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
#include <svd/dataobjects/SVDShaperDigit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <arich/dataobjects/ARICHDigit.h>
#include <bklm/dataobjects/BKLMDigit.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <background/dataobjects/BackgroundInfo.h>

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
    addParam("PXDDigitsName", m_PXDDigitsName,
             "name of PXD collection to overlay with BG", string(""));
    addParam("SVDShaperDigitsName", m_SVDShaperDigitsName,
             "name of SVD collection to overlay with BG", string(""));
    addParam("CDCHitsName", m_CDCHitsName,
             "name of CDC collection to overlay with BG", string(""));
    addParam("TOPDigitsName", m_TOPDigitsName,
             "name of TOP collection to overlay with BG", string(""));
    addParam("ARICHDigitsName", m_ARICHDigitsName,
             "name of ARICH collection to overlay with BG", string(""));
    addParam("BKLMDigitsName", m_BKLMDigitsName,
             "name of BKLM collection to overlay with BG", string(""));
    addParam("EKLMDigitsName", m_EKLMDigitsName,
             "name of EKLM collection to overlay with BG", string(""));

  }

  BGOverlayExecutorModule::~BGOverlayExecutorModule()
  {
  }

  void BGOverlayExecutorModule::initialize()
  {
    // get name of extension that is used in BGOverlayInput for BG collections
    StoreObjPtr<BackgroundInfo> bkgInfo("", DataStore::c_Persistent);
    if (bkgInfo.isValid()) {
      if (bkgInfo->getMethod() == BackgroundInfo::c_Overlay) {
        m_extensionName = bkgInfo->getExtensionName();
      } else {
        B2ERROR("BGOverlayExecutor: no BGOverlayInput module in the path");
      }
    } else {
      B2ERROR("BGOverlayExecutor: no BGOverlayInput module in the path");
    }

    // registration in datastore (all as optional input - see template function)
    registerDigits<PXDDigit>(m_PXDDigitsName);
    registerDigits<SVDShaperDigit>(m_SVDShaperDigitsName);
    registerDigits<CDCHit>(m_CDCHitsName);
    registerDigits<TOPDigit>(m_TOPDigitsName);
    registerDigits<ARICHDigit>(m_ARICHDigitsName);
    registerDigits<BKLMDigit>(m_BKLMDigitsName);
    registerDigits<EKLMDigit>(m_EKLMDigitsName);

  }

  void BGOverlayExecutorModule::beginRun()
  {
  }

  void BGOverlayExecutorModule::event()
  {
    /* note: dataobject must inherit from DigitBase */

    addBGDigits<PXDDigit>(m_PXDDigitsName);
    addBGDigits<SVDShaperDigit>(m_SVDShaperDigitsName);
    addBGDigits<CDCHit>(m_CDCHitsName);
    addBGDigits<TOPDigit>(m_TOPDigitsName);
    addBGDigits<ARICHDigit>(m_ARICHDigitsName);
    //Compressed waveforms are loaded to the datastore by BGOverlayInputModule and unpacked and overlayed in ECLDigitizerModule
    addBGDigits<BKLMDigit>(m_BKLMDigitsName);
    addBGDigits<EKLMDigit>(m_EKLMDigitsName);

  }


  void BGOverlayExecutorModule::endRun()
  {
  }

  void BGOverlayExecutorModule::terminate()
  {
  }


} // end Belle2 namespace


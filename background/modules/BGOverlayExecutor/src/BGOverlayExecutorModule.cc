/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <background/modules/BGOverlayExecutor/BGOverlayExecutorModule.h>

// framework - DataStore
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// detector Digits, Clusters or waveforms
#include <vtx/dataobjects/VTXDigit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <arich/dataobjects/ARICHDigit.h>
#include <klm/dataobjects/KLMDigit.h>
#include <framework/dataobjects/BackgroundInfo.h>

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
    addParam("bkgInfoName", m_BackgroundInfoInstanceName, "name of the BackgroundInfo StoreObjPtr", string(""));
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
    addParam("VTXDigitsName", m_VTXDigitsName,
             "name of VTX collection to overlay with BG", string(""));
    addParam("KLMDigitsName", m_KLMDigitsName,
             "name of KLM collection to overlay with BG", string(""));
    addParam("components", m_components,
             "Detector components to be included in overlay (empty list means all)", m_components);
  }


  void BGOverlayExecutorModule::initialize()
  {
    // get name of extension that is used in BGOverlayInput for BG collections
    StoreObjPtr<BackgroundInfo> bkgInfo(m_BackgroundInfoInstanceName, DataStore::c_Persistent);
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
    registerDigits<VTXDigit>(m_VTXDigitsName);
    registerDigits<KLMDigit>(m_KLMDigitsName);

    // toggle ON the components from the list
    if (m_components.empty()) {
      m_addPXD = true;
      m_addSVD = true;
      m_addCDC = true;
      m_addTOP = true;
      m_addARICH = true;
      m_addKLM = true;
      m_addVTX = true;
    } else {
      for (const auto& component : m_components) {
        if (component == "PXD") m_addPXD = true;
        else if (component == "SVD") m_addSVD = true;
        else if (component == "CDC") m_addCDC = true;
        else if (component == "TOP") m_addTOP = true;
        else if (component == "ARICH") m_addARICH = true;
        else if (component == "ECL") continue; // not relevant - overlay implemented in ECLDigitizer
        else if (component == "KLM") m_addKLM = true;
        else if (component == "VTX") m_addVTX = true;
        else B2ERROR("Unknown detector component '" << component << "'");
      }
    }

  }


  void BGOverlayExecutorModule::event()
  {
    /* note: dataobject must inherit from DigitBase */

    if (m_addPXD) addBGDigits<PXDDigit>(m_PXDDigitsName);
    if (m_addSVD) addBGDigits<SVDShaperDigit>(m_SVDShaperDigitsName);
    if (m_addCDC) addBGDigits<CDCHit>(m_CDCHitsName);
    if (m_addTOP) addBGDigits<TOPDigit>(m_TOPDigitsName);
    if (m_addARICH) addBGDigits<ARICHDigit>(m_ARICHDigitsName);
    //Compressed waveforms are loaded to the datastore by BGOverlayInputModule and unpacked and overlayed in ECLDigitizerModule
    if (m_addKLM) addBGDigits<KLMDigit>(m_KLMDigitsName);
    if (m_addVTX) addBGDigits<VTXDigit>(m_VTXDigitsName);
  }

} // end Belle2 namespace


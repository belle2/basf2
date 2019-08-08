/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <pxd/dataobjects/PXDDAQStatus.h>
#include <vxd/geometry/GeoCache.h>


#include <framework/datastore/StoreObjPtr.h>
#include <TH2.h>
#include <TH1.h>
#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  namespace PXD {
    /** The PXD DAQ DQM module.
     *
     * Creates basic DQM for PXD DAQ errors and statistics
     */
    class PXDDAQDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      PXDDAQDQMModule();

    private:
      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

      /** Input array for DAQ Status. */
      StoreObjPtr<PXDDAQStatus> m_storeDAQEvtStats;

      //the geometry
      VXD::GeoCache& m_vxdGeometry;

      /// Remark: Because of DHH load balancing and sub event building,
      /// the very same DHE and DHC can show up in different packets (for different events)!
      /// but we will fill only one histogram
      TH1F* hDAQErrorEvent{};          /** per event errors */
      TH1F* hDAQUseableModule{};          /** Count Useable/unuseable decision */
      TH1F* hDAQNotUseableModule{};          /** Count Useable/unuseable decision */
      // TH1F* hDAQErrorPacket{};         /** per packet (event builder input) errors  */
      TH2F* hDAQErrorDHC{};          /** individual DHC errors  */
      TH2F* hDAQErrorDHE{};          /** individual DHE errors  */
      TH2F* hDAQEndErrorDHC{};  /** individual DHC END errors  */
      TH2F* hDAQEndErrorDHE{}; /** individual DHE END errors  */
      std::map<VxdID, TH1F*> hDAQDHETriggerGate;/** DHE Trigger Gate ("start Row")  */
      std::map<VxdID, TH1F*> hDAQDHEReduction;/** DHE data reduction  */
      std::map<VxdID, TH2F*> hDAQCM;/** Common Mode per DHE to gate and DHP level */
      std::map<VxdID, TH1F*> hDAQCM2;/** Common Mode per DHE to gate and DHP level */
      std::map<int, TH1F*> hDAQDHCReduction;/** DHC data reduction  */

      void initialize() override final;

      void beginRun() override final;

      void event() override final;

      void defineHisto() override final;

      std::vector<std::string> err;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

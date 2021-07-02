/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <pxd/dataobjects/PXDDAQStatus.h>
#include <vxd/geometry/GeoCache.h>
#include <rawdata/dataobjects/RawSVD.h>

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

      /** Input array for SVD/x-check HLT EODB . */
      StoreArray<RawSVD> m_rawSVD;

      /** Input array for TTD/FTSW */
      StoreArray<RawFTSW> m_rawTTD;

      /** the geometry */
      VXD::GeoCache& m_vxdGeometry;

      /// Remark: Because of DHH load balancing and sub event building,
      /// the very same DHE and DHC can show up in different packets (for different events)!
      /// but we will fill only one histogram
      TH1D* hDAQErrorEvent{};          /**< per event errors */
      TH1D* hDAQUseableModule{};          /**< Count Useable/unuseable decision */
      TH1D* hDAQNotUseableModule{};          /**< Count Useable/unuseable decision */
      TH1D* hDAQDHPDataMissing{};          /**< Count Missing DHP data */
      // TH1F* hDAQErrorPacket{};         /**< per packet (event builder input) errors  */
      TH2D* hDAQErrorDHC{};          /**< individual DHC errors  */
      TH2D* hDAQErrorDHE{};          /**< individual DHE errors  */
      TH2D* hDAQEndErrorDHC{};  /**< individual DHC END errors  */
      TH2D* hDAQEndErrorDHE{}; /**< individual DHE END errors  */
      std::map<VxdID, TH1D*> hDAQDHETriggerGate;/**< DHE Trigger Gate ("start Row")  */
      std::map<VxdID, TH1D*> hDAQDHEReduction;/**< DHE data reduction  */
      std::map<VxdID, TH2D*> hDAQCM;/**< Common Mode per DHE to gate and DHP level */
      std::map<VxdID, TH1D*> hDAQCM2;/**< Common Mode per DHE to gate and DHP level */
      std::map<int, TH1D*> hDAQDHCReduction;/**< DHC data reduction  */

      TH1I* hEODBAfterInjLER{};          /**< Histogram of EODB after LER injection */
      TH1I* hEODBAfterInjHER{};          /**< Histogram of EODB after HER injection */
      TH1I* hCM63AfterInjLER{};          /**< Histogram of CM63 after LER injection */
      TH1I* hCM63AfterInjHER{};          /**< Histogram of CM63 after HER injection */
      TH1I* hTruncAfterInjLER{};          /**< Histogram Truncation after LER injection */
      TH1I* hTruncAfterInjHER{};          /**< Histogram Truncation after HER injection */
      TH1I* hMissAfterInjLER{};          /**< Histogram MissFrame after LER injection */
      TH1I* hMissAfterInjHER{};          /**< Histogram MissFrame after HER injection */
      TH1I* hEODBTrgDiff{};          /**< Histogram of EODB after  last trigger */
      TH1I* hCM63TrgDiff{};          /**< Histogram of CM63 after  last trigger */
      TH1I* hTruncTrgDiff{};          /**< Histogram Truncation after  last trigger */
      TH1I* hMissTrgDiff{};          /**< Histogram MissFrame after  last trigger */
      TH1D* hDAQStat{};                 /**< Histogram for Truncation etc Stats */

      /** Initialize */
      void initialize() override final;

      /** Begin run */
      void beginRun() override final;

      /** Event */
      void event() override final;

      /** Define histograms */
      void defineHisto() override final;

      /** vector of errors */
      std::vector<std::string> err;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

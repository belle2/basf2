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
#include <vxd/geometry/GeoCache.h>
//#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDRawROIs.h>
#include <TH2.h>
#include <string>
#include <map>

namespace Belle2 {

  namespace PXD {
    /** The raw PXD DQM module.
     *
     * Creates basic DQM for ROIs from PXD data stream
     */
    class PXDROIDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      PXDROIDQMModule();

    private:
      void initialize() override final;

      void beginRun() override final;

      void event() override final;

      void defineHisto() override final;

    private:
      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

      std::string m_PXDRawROIsName;        /**< RawROI StoreArray name */

      bool m_eachModule{false};// create a histo per module

      /** Storearray for ROIs  */
      StoreArray<PXDRawROIs> m_storeROIs{};

      /** the VXD geometry */
      VXD::GeoCache& m_vxdGeometry;

      /** Histogram 2d hitmap */
      TH1F* hrawROIcount = nullptr;     /** Histogram  */
      TH1F* hrawROItype = nullptr;     /** Histogram  */

      std::map <int, TH2F*> hrawROIHLTmapModule{};     /** Histogram  */
      std::map <int, TH2F*> hrawROIHLTsizeModule{};     /** Histogram  */
      std::map <int, TH2F*> hrawROIDCmapModule{};     /** Histogram  */
      std::map <int, TH2F*> hrawROIDCsizeModule{};     /** Histogram  */

      TH2F* hrawROIHLTmap = nullptr;      /** Histogram  */
      TH2F* hrawROIHLTsize = nullptr;      /** Histogram  */
      TH1F* hrawROIHLT_DHHID = nullptr;     /** Histogram  */
      TH1F* hrawROIHLTminU = nullptr;     /** Histogram  */
      TH1F* hrawROIHLTmaxU = nullptr; /** Histogram  */
      TH1F* hrawROIHLTminV = nullptr; /** Histogram  */
      TH1F* hrawROIHLTmaxV = nullptr; /** Histogram  */
      TH1F* hrawROIHLTsizeV = nullptr; /** Histogram  */
      TH1F* hrawROIHLTsizeU = nullptr; /** Histogram  */

      TH2F* hrawROIDCmap = nullptr;      /** Histogram  */
      TH2F* hrawROIDCsize = nullptr;      /** Histogram  */
      TH1F* hrawROIDC_DHHID = nullptr;     /** Histogram  */
      TH1F* hrawROIDCminU = nullptr;     /** Histogram  */
      TH1F* hrawROIDCmaxU = nullptr; /** Histogram  */
      TH1F* hrawROIDCminV = nullptr; /** Histogram  */
      TH1F* hrawROIDCmaxV = nullptr; /** Histogram  */
      TH1F* hrawROIDCsizeV = nullptr; /** Histogram  */
      TH1F* hrawROIDCsizeU = nullptr; /** Histogram  */

      TH2F* hrawROINrDCvsNrHLT = nullptr; /** Histogram  */
      TH2F* hrawROIEVTsWithOneSource = nullptr; /** Histogram  */

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2


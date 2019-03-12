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
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDRawROIs.h>
#include <string>
#include <TH2.h>

namespace Belle2 {

  namespace PXD {
    /** The raw PXD DQM module.
     *
     * Creates basic DQM for Raw PXD data and Pixels
     */
    class PXDROIDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      PXDROIDQMModule();

      // virtual ~PXDROIDQMModule();

    private:
      void initialize() override final;

      void beginRun() override final;

      void event() override final;

      void defineHisto() override final;

    private:
      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

      std::string m_PXDRawROIsName;        /**< RawROI StoreArray name */

      /** Storearray for ROIs  */
      StoreArray<PXDRawROIs> m_storeROIs;

      /** Histogram 2d hitmap */
      TH1F* hrawROIcount = nullptr;     /** Histogram  */
      TH1F* hrawROItype = nullptr;     /** Histogram  */

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

      //map<int,int> sensor_to_histindex;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2


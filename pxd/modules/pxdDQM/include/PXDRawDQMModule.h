/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach, Bjoern Spruck                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDRawAdc.h>
#include <pxd/dataobjects/PXDDAQStatus.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <string>
#include <TH2.h>

namespace Belle2 {

  namespace PXD {
    /** The raw PXD DQM module.
     *
     * Creates basic DQM for Raw PXD data and Pixels
     */
    class PXDRawDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      PXDRawDQMModule();

      // virtual ~PXDRawDQMModule();

    private:
      /** Initialize */
      void initialize() override final;

      /** Begin run */
      void beginRun() override final;

      /** Event */
      void event() override final;

      /** Define histograms */
      void defineHisto() override final;


    private:

      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

      std::string m_storeRawPxdrarrayName;        /**< RawPXD StoreArray name */
      std::string m_storeRawHitsName;             /**< PXDRawHits StoreArray name */
      std::string m_storeRawAdcsName;             /**< RawAdcs StoreArray name */

      /** Storearray for raw data packets  */
      StoreArray<RawPXD> m_storeRawPxdrarray;
      /** Storearray for raw pixels   */
      StoreArray<PXDRawHit> m_storeRawHits;
      /** Storearray for ADC from full frames  */
      StoreArray<PXDRawAdc> m_storeRawAdcs;
      /** Input array for DAQ Status. */
      StoreObjPtr<PXDDAQStatus> m_storeDAQEvtStats;

      /** Histogram number of raw packets */
      TH1F* hrawPxdPackets = nullptr;
      /** Histogram raw packet size */
      TH1F* hrawPxdPacketSize = nullptr;
      /** Histogram pixelcount/??? */
      TH1F* hrawPxdHitsCount = nullptr;
      /** Histogram 2d hitmap (all)*/
      TH2F* hrawPxdHitMapAll = nullptr;
      /** Histogram Adc 2d hitmap (full frames only) (all pxd) */
      TH2F* hrawPxdAdcMapAll = nullptr;
      /** Histogram 2d hitmap */
      TH2F* hrawPxdHitMap[64] = {};
      /** Histogram 2d chargemap */
      TH2F* hrawPxdChargeMap[64] = {};
      /** Histogram raw pixel charge */
      TH1F* hrawPxdHitsCharge[64] = {};
      /** Histogram raw pixel hit "time" window */
      TH1F* hrawPxdHitTimeWindow[64] = {};
      /** Histogram raw pixel trigger gate window */
      TH1F* hrawPxdGateTimeWindow[64] = {};

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2


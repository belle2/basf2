/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach, Bjoern Spruck                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDRawDQMModule_H
#define PXDRawDQMModule_H

#include <framework/core/HistoModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <string>
//#include <map>
#include <TH2.h>

namespace Belle2 {

  namespace PXD {
    /** \addtogroup modules
    * @{
    */
    /** The raw PXD DQM module.
     *
     * Creates basic DQM for Raw PXD data and Pixels
     */
    class PXDRawDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      PXDRawDQMModule();

      // virtual ~PXDRawDQMModule();

      virtual void initialize();

      virtual void beginRun();

      virtual void event();

    private:

      /** Storearray for raw data packets  */
      StoreArray<RawPXD> m_storeRawPxdrarray;
      /** Storearray for raw pixels   */
      StoreArray<PXDRawHit> m_storeRawHits;

      /** Histogram number of raw packets */
      TH1F* hrawPxdPackets;
      /** Histogram raw packet size */
      TH1F* hrawPxdPacketSize;
      /** Histogram pixelcount/??? */
      TH1F* hrawPxdHitsCount;
      /** Histogram 2d hitmap */
      TH2F* hrawPxdHits[10];
      /** Histogram raw pixel charge */
      TH1F* hrawPxdHitsCharge[10];
      /** Histogram raw pixel common mode */
      TH1F* hrawPxdHitsCommonMode[10];
      /** Histogram raw pixel trigger window */
      TH1F* hrawPxdHitsTimeWindow[10];

      //map<int,int> sensor_to_histindex;

      virtual void defineHisto();

    };//end class declaration

    /** @}*/

  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDRawDQMModule_H

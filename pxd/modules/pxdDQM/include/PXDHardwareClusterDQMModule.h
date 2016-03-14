/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach, Bjoern Spruck                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDHARDWARECLUSTERDQMMODULE_H
#define PXDHARDWARECLUSTERDQMMODULE_H

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif
#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDHardwareCluster.h>
#include <pxd/dataobjects/PXDRawCluster.h>
#include <string>
#include <TH1F.h>
#include <TH2F.h>

#include <framework/core/HistoModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDRawROIs.h>
#include <string>
#include <TH2.h>
#include <TH1.h>
#include <TH3.h>
#include "TObject.h"

namespace Belle2 {

  namespace PXD {
    /** The raw PXD DQM module.
     *
     * Creates basic DQM for Raw PXD data and Pixels
     */
    class PXDHardwareClusterDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      PXDHardwareClusterDQMModule();

      /** Default Constructor */
      //virtual ~PXDHardwareClusterDQMModule();

      virtual void initialize();

      virtual void beginRun();

      virtual void event();

    private:

      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

      /** Storearray for raw data packets  */
      StoreArray<PXDHardwareCluster> m_storeHardClusterArray;
      /** Storearray for raw pixels   */
      StoreArray<PXDRawCluster> m_storeRawCluster;

      /** Histogram number of hardware cluster packets */
      TH1F* hHardClusterPerHalfLadder;
      /** Histogram hardware cluster packet size */
      TH1F* hHardClusterPacketSize;
      /** Histogram pixel in cluster count/??? */
      TH1F* hHardClusterHitsCount;
      /** Histogram 2d hitmap (all)*/
      TH2F* hHardClusterHitMapAll;
      /** Histogram Adc of cluster pixels */
      TH1F* hHardClusterPixelAdc;
      /** Histogram Adc of full cluster */
      TH1F* hHardClusterAdc;
      /** Histogram Adc of seed pixels */
      TH1F* hHardClusterSeedAdc;


      virtual void defineHisto();

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDHardwareClusterDQMModule_H

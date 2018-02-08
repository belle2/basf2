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
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <string>
//#include <map>
#include <TH2.h>

namespace Belle2 {

  namespace PXD {
    /** The raw PXD DQM module.
     *
     * Creates basic DQM for Raw PXD data and Pixels
     */
    class PXDRawDQMChipsModule : public HistoModule {
      enum {eNumSwitcher = 6, eNumDCD = 4, eNumSensors = 64};

    public:

      /** Constructor defining the parameters */
      PXDRawDQMChipsModule();

      // virtual ~PXDRawDQMChipsModule();

      virtual void initialize();

      virtual void beginRun();

      virtual void event();

    private:

      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

      std::string m_storeRawHitsName;             /**< PXDRawHits StoreArray name */

      /** Storearray for raw pixels   */
      StoreArray<PXDRawHit> m_storeRawHits;

      /** Histogram pixelcount/??? */
      TH1F* hrawPxdHitsCount[eNumSensors][eNumSwitcher][eNumDCD];
      /** Histogram raw pixel charge */
      TH1F* hrawPxdHitsCharge[eNumSensors][eNumSwitcher][eNumDCD];

      virtual void defineHisto();

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2


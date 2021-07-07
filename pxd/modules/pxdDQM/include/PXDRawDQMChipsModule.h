/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDRawHit.h>

#include <TH1F.h>

#include <string>

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

      /** Initialize */
      virtual void initialize() override;

      /** Begin run */
      virtual void beginRun() override;

      /** Event */
      virtual void event() override;

    private:

      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

      std::string m_storeRawHitsName;             /**< PXDRawHits StoreArray name */

      /** Storearray for raw pixels   */
      StoreArray<PXDRawHit> m_storeRawHits;

      /** Histogram pixelcount/??? */
      TH1F* hrawPxdHitsCount[eNumSensors][eNumSwitcher][eNumDCD] = {};
      /** Histogram raw pixel charge */
      TH1F* hrawPxdHitsCharge[eNumSensors][eNumSwitcher][eNumDCD] = {};

      /** Define histograms */
      virtual void defineHisto() override;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2


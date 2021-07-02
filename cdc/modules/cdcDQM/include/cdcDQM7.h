/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRawHit.h>

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <rawdata/dataobjects/RawFTSW.h>

#include <vector>
#include "TH1F.h"
#include "TH1D.h"
#include "TH2D.h"

namespace Belle2 {

  /**
   * The module for Data Quality Monitor.
   */
  class cdcDQM7Module : public HistoModule {

  public:

    //! Constructor
    cdcDQM7Module();

    //! Destructor
    virtual ~cdcDQM7Module();

    //! Module functions
    void initialize() override;
    //! action at runbegin
    void beginRun() override;
    //! action per each event
    void event() override;
    //! action at run end
    void endRun() override;
    //! termination action
    void terminate() override;

    //! function to define histograms
    void defineHisto() override;

  private:

    //! CDC raw hits.
    StoreArray<CDCRawHit> m_CDCRawHits;

    //! CDC hits.
    StoreArray<CDCHit> m_CDCHits;

    TH1D* h_nhits_L[56] = {nullptr}; /**<histogram hit in each layer*/

    TH1D* h_tdc_sL[9] = {nullptr};/**<tdc each super layer*/
    TH1D* h_adc_sL[9] = {nullptr};/**<adc each super layer*/

    TH1D* h_fast_tdc = nullptr;/**<fastest TDC in each event*/
    TH1D* h_board_out_tdc = nullptr;/**<board w/ out of range TDC*/

    TH2D* bmap_2 = nullptr;/**<board status map 2D*/
    // add
    TH1D* h_occ = nullptr;/**<occupancy*/
    TH1D* h_occ_L[56] = {nullptr}; /**<occupancy-layer-dep.*/

    TH1D* h_hit_cell = nullptr;/**<hits-of-cell*/

    TH1D* h_EoccAfterInjLER = nullptr; /**<nhits after LER injection. */
    TH1D* h_EoccAfterInjHER = nullptr; /**<nhits after HER injection. */
    TH1D* h_occAfterInjLER = nullptr; /**<occupancy after LER injection. */
    TH1D* h_occAfterInjHER = nullptr; /**<occupancy after HER injection. */

    StoreArray<RawFTSW> m_rawFTSW; /**< Input array for DAQ Status. */

  };
}

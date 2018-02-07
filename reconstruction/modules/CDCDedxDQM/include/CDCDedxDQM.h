/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <reconstruction/dataobjects/CDCDedxTrack.h>

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>

#include "TH1F.h"
#include "TF1.h"

namespace Belle2 {

  /** Extracts dE/dx information for calibration testing. Writes a ROOT file.
   */
  class CDCDedxDQMModule : public HistoModule {

  public:

    /** Default constructor */
    CDCDedxDQMModule();

    /** Destructor */
    virtual ~CDCDedxDQMModule();

    /** Initialize the module */
    virtual void initialize();

    /** This method is called for each run */
    virtual void beginRun();

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event();

    /** This method is called at the end of each run */
    virtual void endRun();

    /** End of the event processing. */
    virtual void terminate();

    /** Function to define histograms. */
    virtual void defineHisto();

  private:

    /** Store array: CDCDedxTrack */
    StoreArray<CDCDedxTrack> m_cdcDedxTracks;

    TH1F* m_h_dedx = nullptr; /**< Histogram for dE/dx truncated means */
    TH1F* m_h_dedxmean = nullptr; /**< Histogram for average dE/dx mean */
    TH1F* m_h_dedxsigma = nullptr; /**< Histogram for dE/dx resolution */

  };
} // Belle2 namespace

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

#include <framework/core/HistoModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBArray.h>

#include <reconstruction/dbobjects/CDCDedxWireGain.h>
#include <reconstruction/dbobjects/CDCDedxRunGain.h>
#include <reconstruction/dbobjects/CDCDedxCosine.h>

#include <string>
#include <vector>
#include <map>
#include "TH1F.h"

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

    TH1F* m_h_rungains = nullptr; /**< Histogram for run gains */
    TH1F* m_h_wiregains = nullptr; /**< Histogram for wire gains */
    TH1F* m_h_cosinegains = nullptr; /**< Histogram for electron saturation constants */

    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB objects */
    DBObjPtr<CDCDedxRunGain> m_DBRunGain; /**< Run gain DB objects */

    DBObjPtr<CDCDedxCosine> m_DBCosine; /**< Electron saturation correction DB objects */
    std::map<double, double> m_cosineGains;  /**< Electron saturation correction constants */

  };
} // Belle2 namespace

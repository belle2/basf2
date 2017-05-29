/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>
#include <reconstruction/dataobjects/CDCDedxTrack.h>

#include <framework/datastore/StoreArray.h>

#include <TTree.h>
#include <vector>

namespace Belle2 {
  /**
   * A collector module for CDC dE/dx wire gain calibration
   *
   */
  class CDCDedxWireGainCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    CDCDedxWireGainCollectorModule();

    /**
     * Create and book ROOT objects
     */
    virtual void prepare();

    /**
     * Fill ROOT objects
     */
    virtual void collect();


  private:
    // module params
    int m_maxNumHits; /**< maximum acceptable number of hits per track */

    // track level information
    double m_dedx = -1;  /**< dE/dx truncated mean */
    double m_costh = -1; /**< track cos(theta) */
    int m_nhits = -1;    /**< number of dE/dx hits on the track */

    // hit level information
    std::vector<int> m_wire;       /**< wire number for hit */
    std::vector<int> m_layer;      /**< continuous layer number for hit */
    std::vector<double> m_dedxhit; /**< dE/dx for hit */
  };
}

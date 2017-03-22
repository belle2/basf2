/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCDEDXWIREGAINCOLLECTORMODULE_H
#define CDCDEDXWIREGAINCOLLECTORMODULE_H

#include <calibration/CalibrationCollectorModule.h>
#include <reconstruction/dataobjects/CDCDedxTrack.h>

#include <framework/pcore/ProcHandler.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>

#include <TTree.h>
#include <TFile.h>
#include <TRandom.h>


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

    // event level information
    int m_evt = -1;    /**< Current event id */
    int m_run = -1;    /**< Current run id */
    int m_exp = -1;    /**< Current experiment id */
    int m_procId = -1; /**< Current process id */

    // track level information
    double m_dedx = -1;  /**< dE/dx truncated mean */
    double m_costh = -1; /**< track cos(theta) */
    int m_nhits = -1;    /**< number of dE/dx hits on the track */

    // hit level information
    int m_wire[100];       /**< wire number for hit */
    int m_layer[100];      /**< continuous layer number for hit */
    double m_dedxhit[100]; /**< dE/dx for hit */
  };
}

#endif

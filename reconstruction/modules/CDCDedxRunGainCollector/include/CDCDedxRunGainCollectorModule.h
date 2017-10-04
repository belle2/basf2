/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCDEDXRUNGAINCOLLECTORMODULE_H
#define CDCDEDXRUNGAINCOLLECTORMODULE_H

#include <calibration/CalibrationCollectorModule.h>
#include <reconstruction/dataobjects/CDCDedxTrack.h>

#include <framework/pcore/ProcHandler.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>

#include <TFile.h>
#include <TRandom.h>


namespace Belle2 {
  /**
   * A collector module for CDC dE/dx run gains calibration
   *
   */
  class CDCDedxRunGainCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    CDCDedxRunGainCollectorModule();

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
  };
}

#endif

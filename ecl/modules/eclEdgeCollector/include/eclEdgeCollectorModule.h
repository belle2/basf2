/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Contributors: Christopher Heary (hearty@physics.ubc.ca)                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>
#include <framework/database/DBObjPtr.h>

namespace Belle2 {
  class ECLCrystalCalib;


  /** Collector to store ECL crystal locations */
  class eclEdgeCollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor: Sets the description, the properties and the parameters of the module */
    eclEdgeCollectorModule();

    /** Define histograms and read payloads from DB */
    virtual void prepare() override;

    /** Accumulate histograms */
    virtual void collect() override;

  private:
    bool storeGeo = true; /**< force geometry to be saved first event */

    /** Offset between crystal center and lower edge, from database */
    DBObjPtr<ECLCrystalCalib> m_ECLCrystalOffsetTheta; /**< offset in theta */
    DBObjPtr<ECLCrystalCalib> m_ECLCrystalOffsetPhi; /**< offset in phi */

  };
}


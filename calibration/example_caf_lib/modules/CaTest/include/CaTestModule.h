/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <string>

namespace Belle2 {
  /// Testing module for collection of calibration data
  class CaTestModule : public CalibrationCollectorModule {

  public:

    /// Constructor: Sets the description, the properties and the parameters of the module.
    CaTestModule();

    virtual void prepare() override;
    virtual void collect() override;
    virtual void startRun() override;
    virtual void closeRun() override;
    virtual void finish() override;
  private:

    StoreObjPtr<EventMetaData> m_emdata; /**< emdata */

    int m_evt = -1; /**< Current event id */
    int m_run = -1; /**< Current run id */
    int m_exp = -1; /**< Current experiment id */

    double m_hitX = 0.0; /**< Fake x coordinate of a hit */
    double m_hitY = 0.0; /**< Fake y coordinate of a hit */
    double m_hitZ = 0.0; /**< Fake z coordinate of a hit */

    double m_trackX = 0.0; /**< trackX */
    double m_trackY = 0.0; /**< trackY */
    double m_trackZ = 0.0; /**< trackZ */
    double m_chisq = 0.0; /**< chisq */
    double m_pvalue = 0.0; /**< pvalue */
    double m_dof = 0.0; /**< dof */

    /** Number of entries created in the saved tree per event. Will affect the total size of objects stored */
    int m_entriesPerEvent;
    /** Spread of gaussian (mean=42) filling test histogram (range=<0,100>) - probability of algo iterations depend on it */
    int m_spread;
    /// Runs during the defineHisto() function
    virtual void inDefineHisto() override;
    /// Describe process (gives debug output)
    void describeProcess(const std::string& functionName);
  };
}

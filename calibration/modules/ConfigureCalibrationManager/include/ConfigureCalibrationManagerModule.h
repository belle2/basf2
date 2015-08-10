/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CONFIGURECALIBRATIONMANAGERMODULE_H
#define CONFIGURECALIBRATIONMANAGERMODULE_H

#include <framework/core/Module.h>

using namespace std;
namespace Belle2 {
  /**
   * Internal module to to configure Calibration Manager
   *
   * Internal module to to configure Calibration Manager
   *
   */
  class ConfigureCalibrationManagerModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ConfigureCalibrationManagerModule();

    /** Configure the manager */
    virtual void initialize();

    /** Save state of the manager */
    virtual void terminate();

    /** Check if manager is already done */
    virtual void beginRun();

  private:

    int m_rangeExpMin;  /**< Range: min Experiment */
    int m_rangeRunMin;  /**< Range: min Run */
    int m_rangeExpMax;  /**< Range: max Experiment */
    int m_rangeRunMax;  /**< Range: max Run */
    string m_stateFile; /**< Name of file with calibration state cache */

    bool m_doCollection;
    bool m_doCalibration;
  };
}

#endif /* CONFIGURECALIBRATIONMANAGERMODULE_H */

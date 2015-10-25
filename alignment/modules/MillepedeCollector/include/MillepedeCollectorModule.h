/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MILLEPEDECOLLECTORMODULE_H
#define MILLEPEDECOLLECTORMODULE_H

#include <calibration/CalibrationCollectorModule.h>


namespace Belle2 {
  /**
   * Calibration data collector for Millepede Algorithm
   *
   * Collects data from GBL-fitted tracks and produces binary files for Millepede
   *
   */
  class MillepedeCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    MillepedeCollectorModule();

    /** Prepration */
    virtual void prepare();

    /** Data collection */
    virtual void collect();

    /** Only for closing mille binaries after each run */
    virtual void endRun();

    /** Make a name for mille binary (encodes module name + starting exp, run and event + process id) */
    std::string getUniqueMilleName();


  private:
    std::string m_tracks;  /**< Name of collection of genfit::Tracks for calibration */
    //std::string m_binary;  /**< Name of Mille binary file with calibration data */

  };
}

#endif /* MILLEPEDECOLLECTORMODULE_H */

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>


namespace Belle2 {
  namespace PXD {

    /** The ActivatePXDGainCalibrator module.
     *
     * This module is responsible reading the calibration constants for PXD
     * gain corrections from the Database.
     */
    class ActivatePXDGainCalibratorModule : public Module {

    public:

      /** Constructor */
      ActivatePXDGainCalibratorModule();

      /** Initialize the module */
      void initialize() override final;

    };  //end class declaration

  }  //end PXD namespace;
}  // end namespace Belle2



/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>


namespace Belle2 {
  namespace VTX {

    /** The ActivateVTXClusterPositionEstimator module.
     *
     * This module is responsible reading the calibration constants for local VTX
     * cluster position estimation from the Database.
     */
    class ActivateVTXClusterPositionEstimatorModule : public Module {

    public:

      /** Constructor */
      ActivateVTXClusterPositionEstimatorModule();

      /** Initialize the module */
      void initialize() override final;

    };  //end class declaration

  }  //end VTX namespace;
}  // end namespace Belle2



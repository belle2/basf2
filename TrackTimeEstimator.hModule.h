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
  /**
   * Computes the track time, defined as the difference between the average of SVD clusters time and the SVDEvent T0
   */
  class TrackTimeEstimator.hModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TrackTimeEstimator.hModule();

    /**  */
    void initialize() override;

    /**  */
    void event() override;


  private:

  };
}

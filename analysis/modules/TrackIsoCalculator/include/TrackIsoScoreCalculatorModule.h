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
   * Define a semi-continuous variable to quantify the isolation level of each standard charged particle. The definition is based on the counting of layers where a nearby track helix is found, as well as on the weight that each sub-detector has on the PID for the given particle hypothesis.
   */
  class TrackIsoScoreCalculatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TrackIsoScoreCalculatorModule();

    /** Destructor */
    ~TrackIsoScoreCalculatorModule() override;

    /** Initialize */
    void initialize() override;

    /** beginRun */
    void beginRun() override;

    /** Event loop */
    void event() override;

    /** endRun */
    void endRun() override;

    /** Terminate */
    void terminate() override;


  private:

    std::vector<std::string> m_detectors;  /**< List of detectors to consider. */
  };
}

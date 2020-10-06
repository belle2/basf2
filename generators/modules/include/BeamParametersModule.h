/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef GENERATORS_MODULES_BEAMPARAMETERSMODULE_H
#define GENERATORS_MODULES_BEAMPARAMETERSMODULE_H

#include <framework/core/Module.h>
#include <framework/dbobjects/BeamParameters.h>
#include <vector>


namespace Belle2 {
  /**
   * Setting of beam parameters
   */
  class BeamParametersModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    BeamParametersModule();

    /** Create the Beamparameters*/
    virtual void initialize() override;

  private:
    BeamParameters m_beamParams; /**< Copy of the beamparameters created from the parameters */
    double m_energyHER;  /**< Energy of the High Energy Ring */
    double m_angleXHER;  /**< theta angle of the High Energy Ring in the x-z plane */
    double m_angleYHER;  /**< theta angle of the High Energy Ring in the y-z plane */
    std::vector<double> m_covHER; /**< Covariance matrix for the High Energy Ring */
    double m_energyLER;  /**< Energy of the Low Energy Ring */
    double m_angleXLER;  /**< theta angle of the Low Energy Ring in the x-z plane */
    double m_angleYLER;  /**< theta angle of the Low Energy Ring in the y-z plane */
    std::vector<double> m_covLER; /**< Covariance matrix for the Low Energy Ring */
    std::vector<double> m_vertex; /**< nominal vertex position */
    std::vector<double> m_covVertex; /**< covariance matrix for the vertex position */
    std::vector<int> m_payloadIov{0, 0, -1, -1}; /**< iov when creating a database payload */

    bool m_smearEnergy; /**< if true, smear energy when generating initial events */
    bool m_smearDirection; /**< if true, smear beam direction when generating initial events */
    bool m_smearVertex; /**< if true, smear vertex position when generating initial events */
    bool m_generateCMS; /**< if true, generate events in CMS, not lab system */
    bool m_createPayload; /**< if true create a new payload with the given parameters */
  };
}

#endif /* BEAMPARAMETERSMODULE_H */

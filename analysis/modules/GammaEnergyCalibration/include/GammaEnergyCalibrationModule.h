/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GAMMAENERGYCALIBRATIONMODULE_H
#define GAMMAENERGYCALIBRATIONMODULE_H

#include <framework/core/Module.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <string>

#include <analysis/dataobjects/Particle.h>

namespace Belle2 {
  /**
   * \addtogroup modules
   * @{ GammaEnergyCalibrationModule @}
   */

  /**
   * Calibrates energy of the photons
   *
   * The module loops over all photon candidates in the specified lists and performs energy calibration. The module overwrites the 4-momentum vector of the photons.   *
   */
  class GammaEnergyCalibrationModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    GammaEnergyCalibrationModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();


    /**
     * Event function
     */
    virtual void event();


  private:
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the particle being selected */

    std::string m_gammaListName;  /**< Input ParticleList name */

    /**
     * Calibrates energy for given photon Particle.
     *
     * The calibration function is:
     * E_cal = E_orig - 0.041
     *
     * Particle's 4-momentum is recalculated iwth the new energy.
     */
    void calibrateEnergy(Particle* particle) const;
  };
}

#endif /* GAMMAENERGYCALIBRATIONMODULE_H */

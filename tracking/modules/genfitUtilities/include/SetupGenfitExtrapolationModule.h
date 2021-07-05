/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <alignment/dbobjects/VXDAlignment.h>

#include <string>

namespace Belle2 {
  /** Setup material handling and magnetic fields for use by genfit's extrapolation code
   *  (RKTrackRep).  This should be one of the first modules on any path working with tracks.
   */
  class SetupGenfitExtrapolationModule : public Module {

  public:
    /** Constructor. */
    SetupGenfitExtrapolationModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    void initialize() override;

  private:
    /** Whether or not this module will raise an error if the geometry is
    * already present. This can be used to add the geometry multiple times if
    * it's not clear if it's already present in another path */
    bool m_ignoreIfPresent = true;

    /// choice of geometry representation: 'TGeo' or 'Geant4'.
    std::string m_geometry = "Geant4";

    /// switch on/off ALL material effects in Genfit. "true" overwrites "true" flags for the individual effects.
    bool m_noEffects = false;
    /// Determines if calculation of energy loss is on/off in Genfit
    bool m_energyLossBetheBloch = true;
    /// Determines if calculation of energy loss variance is on/off in Genfit
    bool m_noiseBetheBloch = true;
    /// Determines if calculation of multiple scattering covariance matrix on/off in Genfit
    bool m_noiseCoulomb = true;
    /// Determines if calculation of bremsstrahlung energy loss is on/off in Genfit
    bool m_energyLossBrems = true;
    /// Determines if calculation of bremsstrahlung energy loss variance is on/off in Genfit
    bool m_noiseBrems = true;
    /// Multiple scattering model
    std::string m_mscModel = "Highland";
    /// Use VXD alignment from database?
    bool m_useVXDAlignment = true;
    /// DB object with VXD alignment
    DBObjPtr<VXDAlignment> m_vxdAlignment;
  };
}

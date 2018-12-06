/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter, Tadeas Bilka                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <alignment/dbobjects/VXDAlignment.h>

#include <string>

#include <Geant4/G4Transform3D.hh>
#include <root/TGeoMatrix.h>

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

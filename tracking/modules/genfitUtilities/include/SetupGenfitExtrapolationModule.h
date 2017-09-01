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
    /** Constructor .
     */
    SetupGenfitExtrapolationModule();

    /** Destructor.
     */
    virtual ~SetupGenfitExtrapolationModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    virtual void initialize();

    /** Called when entering a new run.
     */
    virtual void beginRun();

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    virtual void event();

    /** This method is called if the current run ends.
     */
    virtual void endRun();

    /** This method is called at the end of the event processing.
     */
    virtual void terminate();

  private:

    std::string m_geometry;       /**< choice of geometry representation: 'TGeo' or 'Geant4'.  */

    bool m_noEffects;             /**< switch on/off ALL material effects in Genfit. "true" overwrites "true" flags for the individual effects.*/
    bool m_energyLossBetheBloch;  /**< Determines if calculation of energy loss is on/off in Genfit */
    bool m_noiseBetheBloch;       /**< Determines if calculation of energy loss variance is on/off in Genfit */
    bool m_noiseCoulomb;          /**< Determines if calculation of multiple scattering covariance matrix on/off in Genfit */
    bool m_energyLossBrems;       /**< Determines if calculation of bremsstrahlung energy loss is on/off in Genfit */
    bool m_noiseBrems;            /**< Determines if calculation of bremsstrahlung energy loss variance is on/off in Genfit */
    std::string m_mscModel;       /**< Multiple scattering model */
    bool m_useVXDAlignment;       /**< Use VXD alignment from database? */
    DBObjPtr<VXDAlignment> m_vxdAlignment; /**< DB object with VXD alignment */
  };
}

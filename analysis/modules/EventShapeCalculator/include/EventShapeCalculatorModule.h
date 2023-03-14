/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <Math/Vector3D.h>

#include <framework/core/Module.h>

#include <analysis/dataobjects/EventShapeContainer.h>

#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  /**
   * Module to compute event shape variables starting from three lists of particle objects (tracks, gammas, Klongs).
   * The output is stored in the EventShapeContainer dataobject.
   * This module does not contain the implementation of the core eventshape algorithm, that is rather done in separate
   * basf2 classes. This scheme has been chosen partially because it was already used by the ContinuumSuppression, and partially
   * because different modules may need to access the very same core utilities.
   *
   * The steering parameters of the module mostly consist of flags that enable the user to switch on or off the calculation
   * of different quantities.
   */
  class EventShapeCalculatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    EventShapeCalculatorModule();

    /**
     * Default destructor
     */
    virtual ~EventShapeCalculatorModule() {};

    /** Define the physical parameters. look for them in database. */
    virtual void initialize() override;


    /**
     * Main method, called for each events.
     * It calls the event shape core functions and stores the output in an EventShapeContainer object.
     */
    virtual void event() override;


    /**
     * Turns the ParticleLists provided as inputs in std::vectors of PxPyPzEVector and B2Vector3D,
     * boosting them in the CMS frame.
     */
    int parseParticleLists(std::vector<std::string>);


  private:

    StoreObjPtr<EventShapeContainer> m_eventShapeContainer; /**< event shape container object pointer */

    std::vector<std::string> m_particleListNames;  /**< Names of the ParticleLists (inputs). */
    std::vector<ROOT::Math::PxPyPzEVector>
    m_p4List; /**< vector containing all the 4-momenta of the particles contained in the input lists. */
    std::vector<ROOT::Math::XYZVector>
    m_p3List; /**< vector containing all the 3-momenta of the particles contained in the input lists. */

    bool m_enableThrust = true; /**< Enables the calculation of thust-related quantities.  */
    bool m_enableCollisionAxis = true; /**< Enables the calculation of the quantities related to the collision axis.  */
    bool m_enableFW = true; /**< Enables the calculation of the FW moments. */
    bool m_enableHarmonicMoments = true; /**< Enables the calculation of the Harmonic moments. */
    bool m_enableJets = true; /**< Enables the calculation of the Jet-related quantities. */
    bool m_enableSphericity = true; /**< Enables the calculation of the Sphericity matrix. */
    bool m_enableCleoCones = true; /**< Enables the calculation of the Cleo Cones. */
    bool m_enableAllMoments = false; /**< Enables the calculation of the FW moments from 5 to 8. */
    bool m_checkForDuplicates = false; /**< Enables the check for the duplicates in the input list. */

  };
}


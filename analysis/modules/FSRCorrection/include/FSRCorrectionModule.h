/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Gelb                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FSRCORRECTIONMODULE_H
#define FSRCORRECTIONMODULE_H

#include <framework/core/Module.h>

#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <string>
//#include <vector>
//#include <tuple>
//#include <memory>


namespace Belle2 {
  class Particle;

  /**
   * Final state radiation correction module
   * This module adds the 4 Vector of the (closest) radiative gamma to the 4Vector of a lepton if the specified criteria are fulfilled.
   * It is intended to be used for electrons.
   */
  class FSRCorrectionModule : public Module {

  public:

    /**
     * Constructor
     */
    FSRCorrectionModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    enum {c_DimMatrix = 7};

  private:

    int m_pdgCode;                /**< PDG code of the combined mother particle */

    // fixme do we need this?
    std::string m_decayString;   /**< Input DecayString specifying the decay being reconstructed */
    // fixme do we need this?
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the lepton decay */
    DecayDescriptor m_decaydescriptorGamma; /**< Decay descriptor of the decay being reconstructed */

    std::string m_inputListName; /**< input ParticleList names */
    std::string m_gammaListName; /**< input ParticleList names */
    std::string m_outputListName; /**< output ParticleList name */
    std::string m_outputAntiListName;   /**< output anti-particle list name */

    double m_angleThres; /**< input max angle to be accepted (in degree) */
    double m_energyThres; /**< max energy of gamma to be accepted */

    bool m_writeOut;  /**< toggle output particle list btw. transient/writeOut */

    double m_maxAngle; /**< max angle */

  };

} // Belle2 namespace

#endif

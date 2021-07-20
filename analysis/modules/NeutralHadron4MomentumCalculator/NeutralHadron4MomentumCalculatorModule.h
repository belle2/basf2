/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>
#include <TVector3.h>
#include <TLorentzVector.h>

#include <vector>


namespace Belle2 {
  /**
   * Calculates 4-momentum of a neutral hadron in a given decay chain e.g. B0 -> J/Psi K_L0, or anti-B0 -> p+ K- anti-n0.
   */
  class NeutralHadron4MomentumCalculatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    NeutralHadron4MomentumCalculatorModule();

    /** Iniliazation */
    void initialize() override;

    /** Action to perform for each event */
    void event() override;

    /** Termination */
    void terminate() override;


  private:

    std::string m_listName;  /**< Name of the ParticleList that one wants to perform the calculation */
    StoreObjPtr<ParticleList> m_plist;  /**< ParticleList that one wants to perform the calculation */
  };
}

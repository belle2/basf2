/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sascha Dreyer                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TF1.h>
#include <string>
#include <map>


namespace Belle2 {
  /**
  * "Takes a list of PDG values and lifetime paramters to displaces the vertex of MCParticles with matching PDG value corresponding to the given lifetime parameter. Can be used betwenerator and the detector simulation.
  *
  *
  */
  class GeneratedVertexDisplacerModule : public Module {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    GeneratedVertexDisplacerModule();

    /** Register input and output data, initialises the module */
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;

    /** Terminates the module.  */
    virtual void terminate() override;



  protected:
    // called for a particle, displaces its vertex and of its subsequent daughters
    void displace(MCParticle& particle, float lifetime);
    void displaceDaughter(MCParticle& particle, std::vector<MCParticle*> daughters);
    void getDisplacement(MCParticle& particle, float lifetime, TLorentzVector& displacement);
    std::string m_particleList; /**< The name of the MCParticle collection. */
    StoreArray<MCParticle> m_mcparticles; /**< store array for the MCParticles */
    std::list<int> displaced_particles;


  private:
    // Parameters
    std::string m_lifetimeOption;  /**< Set the lifetime option, either 0=fixed, 1=flat or 2=exponential */
    std::vector<float> m_lifetime;  /**< Set the numerical value of the lifetime c*tau  */
    std::vector<int> m_pdgVals;  /**< Set the particles whose vertices should be displaced  */
    // StoreArray<MCParticle> m_mcParticle; /**<  */

  };

}

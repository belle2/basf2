/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/core/RandomGenerator.h>
#include <framework/gearbox/Const.h>
#include <mdst/dataobjects/MCParticle.h>
#include <TVector3.h>
#include <Math/Vector4D.h>
#include <TF1.h>
#include <string>
#include <map>


namespace Belle2 {
  /**
  * "Takes a list of PDG values and lifetime paramters to displaces the vertex of MCParticles with matching PDG value and their subsequent daughters corresponding to the given lifetime parameter(s). Can be used between the generator and the detector simulation. Lifetime options are "flat", "fixed" and "exponential" where the lifetime(s) should be passed as c*tau in units of [cm]. Furthermore, a "fixedLength" option can be used where the lifetime parameter takes the desired fixed decaylength in units of [cm].
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
    // called for a particle, displaces its vertex and its subsequent daughters
    /** Helper function to displace the mother particles (corresponding to given pdf values). */
    void displace(MCParticle& particle, float lifetime);
    /** Helper function to loop over subsequent daughters and displaces their vertices corresponding to their mother decay vertex. */
    void displaceDaughter(ROOT::Math::PxPyPzEVector& motherDisplacementVector, std::vector<MCParticle*> daughters);
    /** Helper function to calculate the numerical value of the vertex displacement (x,y,z,t) */
    void getDisplacement(const MCParticle& particle, float lifetime, ROOT::Math::PxPyPzEVector& displacement);
    std::string m_particleList; /**< The name of the MCParticle collection. */
    StoreArray<MCParticle> m_mcparticles; /**< store array for the MCParticles */

  private:
    // Parameters
    std::string m_lifetimeOption;  /**< Set the lifetime option, either fixed, flat exponential */
    std::vector<float> m_lifetime;  /**< Set the numerical value of the lifetime ctau [cm]  */
    std::vector<int> m_pdgVals;  /**< Set the particles whose vertices should be displaced  */
    float m_maxDecayTime;  /**< Set the maximal decayTime for the options 'flat' and 'exponential'. */
    bool m_ctau;  /**< Input unit option. True (default): module expects lifetime as ctau [cm]. False: lifetime as tau [ns]. */
  };

}

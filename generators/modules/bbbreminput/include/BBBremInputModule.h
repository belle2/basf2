/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef BBBREMINPUTMODULE_H
#define BBBREMINPUTMODULE_H

#include <framework/core/Module.h>

#include <generators/bbbrem/BBBrem.h>
#include <generators/utilities/InitialParticleGeneration.h>

#include <mdst/dataobjects/MCParticleGraph.h>

namespace Belle2 {

  /**
   * The BBBrem Generator module.
   * Generates low scattering angle radiative Bhabha events (Beam-Beam Bremsstrahlung).
   */
  class BBBremInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    BBBremInputModule();

    /** Destructor. */
    virtual ~BBBremInputModule();

    /** Initializes the module. */
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;

    /** Method is called at the end of the event processing. */
    virtual void terminate() override;


  protected:

    /** Module parameters */
    double m_photonEFrac; /**< Minimum photon energy fraction. */
    bool m_unweighted; /**< True if BBBrem should produce unweighted events. */
    double m_maxWeight; /**< The maximum weight. Used for the event rejection procedure to produce unweighted events. */
    int m_densityCorrectionMode; /**< Mode for bunch density correction. */
    double m_DensityCorrectionParameter; /**< Density correction parameter tc. */

    /** Variables */
    BBBrem m_generator; /**< The BBBrem generator. */
    MCParticleGraph m_mcGraph; /**< The MCParticle graph object. */

  private:

    void initializeGenerator(); /**< Method is called to initialize the generator. */

    bool m_initialized{false}; /**< True if generator has been initialized. */

    DBObjPtr<BeamParameters> m_beamParams; /**< BeamParameter. */

    InitialParticleGeneration m_initial; /**< initial particle used by BeamParameter class */

  };

} // end namespace Belle2


#endif /* BBBREMINPUTMODULE_H */

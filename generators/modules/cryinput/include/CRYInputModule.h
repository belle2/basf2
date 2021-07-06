/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <generators/cry/CRY.h>

#include <mdst/dataobjects/MCParticleGraph.h>

namespace Belle2 {

  /**
   * The CRY Generator module.
   * Generates cosmic events using the CRY generator.
   */
  class CRYInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    CRYInputModule();

    /** Destructor. */
    virtual ~CRYInputModule() = default;

    /** Initializes the module. */
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;

    /** Method is called at the end of the event processing. */
    virtual void terminate() override;

  protected:
    std::string m_cosmicdatadir; /**< cosmic data (used by CRY for interpolation). */
    std::vector<double> m_acceptance; /**< Shape parameters for the acceptance box */
    double m_kineticEnergyThreshold{0.01};  /**< kinetic energy threshold. */
    double m_timeOffset{0};  /**< time offset. */
    int m_maxTrials{10000};  /**< maximum number of trials. */
    double m_boxLength{10000}; /**< length of the sides of the square plane in which to generate cosmics */
    std::string m_date{"1-1-2019"}; /**< date used for generation. */
    bool m_returnGammas{true}; /**< Whether or not CRY should return gammas */
    bool m_returnKaons{true}; /**< Whether or not CRY should return kaons */
    bool m_returnPions{true}; /**< Whether or not CRY should return pions */
    bool m_returnProtons{true}; /**< Whether or not CRY should return protons */
    bool m_returnNeutrons{true}; /**< Whether or not CRY should return neutrons */
    bool m_returnElectrons{true}; /**< Whether or not CRY should return electrons */
    bool m_returnMuons{true}; /**< Whether or not CRY should return muons */

    CRY m_generator;   /**< The CRY generator. */
    MCParticleGraph m_mcGraph; /**< The MCParticle graph object. */
  };


} // end namespace Belle2

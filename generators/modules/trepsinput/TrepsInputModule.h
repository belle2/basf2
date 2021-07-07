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

#include <generators/treps/Treps3B.h>
#include <generators/treps/UtrepsB.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <generators/utilities/InitialParticleGeneration.h>
#include <string>

namespace Belle2 {
  /**
   * Input from TREPS generator for ee->eeff
   */
  class TrepsInputModule : public Module {

  public:
    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TrepsInputModule();

    /** initialization for trepsinput */
    virtual void initialize() override;

    /** initialization for trepsinput */
    virtual void terminate() override;

    /** input event from TREPS */
    virtual void event() override;

    /** Simulate W distribution according to given input file of cross section table */
    double simulateW();

    /** Returns the differential cross section for given W [GeV] */
    double getCrossSection(double W);

    /** Initialize the TREPS generator  */
    void initializeGenerator();

  private:

    /** Treps generator **/
    UtrepsB m_generator;

    /** Parameter file which configures the setting of beam, production particle etc. */
    std::string m_parameterFile;

    /** W-List table file. It has to have two columns, W [GeV] and Number_of_events_at_W */
    std::string m_wListTableFile;

    /** Differential cross section table file. It has to have two columns, W [GeV] and differential_cross_section */
    std::string m_differentialCrossSectionFile;

    MCParticleGraph m_mpg;        /**< An instance of the MCParticle graph. */

    DBObjPtr<BeamParameters> m_beamParams; /**< BeamParameter. */

    InitialParticleGeneration m_initial; /**< initial particle used by BeamParameter class */

    StoreArray<MCParticle> m_mcparticles; /**< MCParticle collection */

    bool m_useDiscreteAndSortedW; /**< if it is true, W-list table is used for discrete and sorted W */

    /**
     * Maximal Q^2 = -q^2, where q is the difference between the initial
     * and final electron or positron momenta.
     */
    double m_maximalQ2;

    /**
     * Maximal |cos(theta)|, where theta is the final-state particle polar
     * angle.
     */
    double m_maximalAbsCosTheta;

    /** Whether to apply cut on |cos(theta)| for charged particles only. */
    bool m_applyCosThetaCutCharged;

    /** Minimal transverse momentum of the final-state particles. */
    double m_minimalTransverseMomentum;

    /**
     * Whether to apply cut on the minimal transverse momentum for
     * charged particles only.
     */
    bool m_applyTransverseMomentumCutCharged;

    bool m_initialized{false}; /**< True if generator has been initialized. */

  };
}


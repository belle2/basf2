/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kiyoshi Hayasaka, Yo Sato                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <generators/treps/UtrepsB.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <generators/utilities/InitialParticleGeneration.h>
#include <string>

namespace Belle2 {
  /**
   * Input from TREPS generator for ee->eeff
   */
  class trepsinputModule : public Module, public UtrepsB {

  public:
    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    trepsinputModule();

    /** initialization for trepsinput */
    virtual void initialize() override;

    /** initialization for trepsinput */
    virtual void terminate() override;

    /** input event from TREPS */
    virtual void event() override;

    std::string rfnfc;

    /** Simulate W distribution according to given input file of cross section table */
    double simulateW();

    /** Returns the differential cross section for given W [GeV] */
    double getCrossSection(double W);

  private:

    MCParticleGraph mpg;        /**< An instance of the MCParticle graph. */

    DBObjPtr<BeamParameters> m_beamParams; /**< BeamParameter. */

    InitialParticleGeneration m_initial; /**< initial particle used by BeamParameter class */

    StoreArray<MCParticle> m_mcparticles; /**< MCParticle collection */

  };
}


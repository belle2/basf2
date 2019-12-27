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
  class trepsinputModule : public Module {

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

    /** Simulate W distribution according to given input file of cross section table */
    double simulateW();

    /** Returns the differential cross section for given W [GeV] */
    double getCrossSection(double W);

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

    bool m_useDiscreteAndSortedW; /* if it is true, W-list table is used for discrete and sorted W */

  };
}


/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Belle2 headers. */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dbobjects/BeamParameters.h>
#include <generators/utilities/InitialParticleGeneration.h>
#include <mdst/dataobjects/MCParticle.h>

/* ROOT headers. */
#include <TLorentzRotation.h>

/* C++ headers. */
#include <string>

namespace Belle2 {
  /**
   * Module for boosting the MCParticles from CM to LAB frame. The module must be appended to the path only when HepMC, Hepevt or LHE input files are used.
   */
  class BoostMCParticlesModule : public Module {

  public:

    /** Constructor. */
    BoostMCParticlesModule();

    /** Initialize. */
    void initialize() override;

    /** Event. */
    void event() override;


  private:

    /** Flag for keeping track of the first call of the event() method. */
    bool m_firstEvent;

    /** Name of the MC particles StoreArray. */
    std::string m_mcParticlesName;

    /** Initial particle generation. */
    InitialParticleGeneration m_initial;

    /** Boost rotation. */
    TLorentzRotation m_boost;

    /** Beam parameters. */
    DBObjPtr<BeamParameters> m_beamParameters;

    /** MC particles. */
    StoreArray<MCParticle> m_mcParticles;

  };
}

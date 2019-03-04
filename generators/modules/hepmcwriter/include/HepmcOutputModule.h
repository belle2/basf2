/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nikolai Hartmann                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <string>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include "HepMC/GenEvent.h"
#include "HepMC/IO_GenEvent.h"
#include "HepMC/IO_HEPEVT.h"

/* #include <generators/hepmc/HepMCReader.h> */
/* #include <generators/utilities/InitialParticleGeneration.h> */

namespace Belle2 {

  /** The HepMCOutput module.
   * Loads events from the MCParticle class and stores the content
   * into an HepMC2 ascii file
   */
  class HepMCOutputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    HepMCOutputModule();

    /** Destructor. */
    virtual ~HepMCOutputModule() {}

    /** Initializes the module. */
    virtual void initialize() override;

    /** method is called for each event. */
    virtual void event() override;

    /** method is called at end */
    virtual void terminate() override;

  protected:

    //Parameter
    std::string m_filename; /**< The output filename. */

    //Variables
    //std::ofstream m_fileStream;  /**< The text file stream. */
    HepMC::IO_HEPEVT m_hepevtio; /**< wrapper/buffer for hepevt conversion. */
    HepMC::IO_GenEvent* m_ascii_io; /**< The text file stream */


  };

} // end namespace Belle2

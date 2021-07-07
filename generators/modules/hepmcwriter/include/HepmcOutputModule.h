/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <string>

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
    bool m_storeVirtualParticles; /**< Flag which specifies if virtual particles are stored in the HEPEvt file. */

    //Variables
    HepMC::IO_HEPEVT m_hepevtio; /**< wrapper/buffer for hepevt conversion. */
    std::unique_ptr<HepMC::IO_GenEvent> m_ascii_io; /**< The text file stream */


  };

} // end namespace Belle2

/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <fstream>
#include <string>

namespace Belle2 {

  /** The HepevtOutput module.
   *
   * Writes the MCParticle collection to a HepEvt file.
   */
  class HepevtOutputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    HepevtOutputModule();

    /** Destructor. */
    virtual ~HepevtOutputModule() {}

    /** Initializes the module. */
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;

    /** Terminates the module. */
    virtual void terminate() override;


  protected:

    //Parameter
    std::string m_filename; /**< The output filename. */
    bool m_mirrorPz; /**< If the directions of HER and LER are switched, mirror Pz. */
    bool m_storeVirtualParticles; /**< Flag which specifies if virtual particles are stored in the HEPEvt file. */
    bool m_fullFormat; /**< Flag which specifies if the full HepEvt format should be written (true), or a compact format (false). */

    //Variables
    std::ofstream m_fileStream;  /**< The text file stream. */

  };

} // end namespace Belle2



/**************************************************************************
 * Belle II detector background library                                   *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HEPEVTOUTPUTMODULE_H
#define HEPEVTOUTPUTMODULE_H

#include <framework/core/Module.h>

#include <iostream>
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
    virtual void initialize();

    /** Method is called for each event. */
    virtual void event();

    /** Terminates the module. */
    virtual void terminate();


  protected:

    //Parameter
    std::string m_filename; /**< The output filename. */
    bool m_mirrorPz; /**< If the directions of HER and LER are switched, mirror Pz. */
    bool m_storeVirtualParticles; /**< Flag to set if virtual particles are stored in the HEPEvt file. */

    //Variables
    std::ofstream m_fileStream;  /**< The text file stream. */

  };

} // end namespace Belle2


#endif /* HEPEVTOUTPUTMODULE_H */

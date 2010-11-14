/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOSAVER_H_
#define GEOSAVER_H_

#include <framework/core/Module.h>

#include <string>


namespace Belle2 {

  /**
   * The GeoSaver module.
   *
   * Takes the current detector geometry from the memory and
   * saves it to a root file.
  */
  class GeoSaverModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description and the parameters of the module.
     */
    GeoSaverModule();

    /** Destructor. */
    virtual ~GeoSaverModule();

    /**
     * Checks the validity of the module parameters.
     * Checks if the filepath of the given filename exists.
     */
    virtual void initialize();

    /** Saves the geometry to a root file for every event. */
    virtual void event();


  protected:


  private:

    std::string m_filenameROOT; /**< The filename of the ROOT file into which the geometry is saved. */

  };
}

#endif /* GEOSAVER_H_ */

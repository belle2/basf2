/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOBUILDERMODULE_H_
#define GEOBUILDERMODULE_H_

#include <framework/core/Module.h>

#include <string>


namespace Belle2 {

  /**
   * The GeoBuilder module.
   *
   * This module creates the detector Belle II detector geometry
   * in memory.
   */
  class GeoBuilderModule : public Module {

  public:

    /**
     * Constructor.
     *
     * Sets the description of the module.
     */
    GeoBuilderModule();

    /** Destructor. */
    virtual ~GeoBuilderModule();

    /*! Creates the geometry during initialization of the module.
     *
     * It calls the method createDetector() of the GeoDetector class which
     * loops over the Creators. The Creators create then the ROOT geometry.
     * In order to access the geometry, the global Variable gGeoManager can
     * be used.
     */
    virtual void initialize();


  protected:


  private:

  };
}

#endif /* GEOBUILDERMODULE_H_ */

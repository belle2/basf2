/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEARREADER_H
#define GEARREADER_H

#include <TGeoMaterial.h>
#include <string>

namespace Belle2 {

  class GearDir;

  /**
   * The GearReader class.
   *
   * Provides convenient methods to read nodes or attributes from a GearDir.
   */
  class GearReader {

  public:

    /**
     * Reads the weight attribute of the node the gearDir is pointing to.
     *
     * @param gearDir The GearDir path from which the weight attribute should be read.
     * @return The weight value which is meant for usage inside a material mixture. If it isn't defined -1 is returned.
     */
    static double readWeightAttribute(GearDir& gearDir);

    /**
     * Reads the name attribute of the node the gearDir is pointing to.
     *
     * @param gearDir The GearDir path from which the name attribute should be read.
     * @return The name value of the specified node. If the name attribute doesn't exist, an empty string is returned.
     */
    static std::string readNameAttribute(GearDir& gearDir);

    /**
     * Reads the unit attribute of the node the gearDir is pointing to.
     *
     * @param gearDir The GearDir path from which the unit attribute should be read.
     * @return The unit value as a string of the specified node. If the unit attribute doesn't exist, an empty string is returned.
     */
    static std::string readUnitAttribute(GearDir& gearDir);

  };

}

#endif /* GEARREADER_H */

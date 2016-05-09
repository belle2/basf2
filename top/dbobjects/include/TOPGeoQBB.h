/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBase.h>

namespace Belle2 {

  /**
   * Geometry parameters of Quartz Bar Box
   */
  class TOPGeoQBB: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoQBB()
    {}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const {return true;} // TODO


  private:

    ClassDef(TOPGeoQBB, 1); /**< ClassDef */

  };

} // end namespace Belle2

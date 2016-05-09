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
   * Geometry parameters of board stack (front-end electronic module)
   */
  class TOPGeoBoardStack: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoBoardStack()
    {}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const {return true;} // TODO


  private:

    ClassDef(TOPGeoBoardStack, 1); /**< ClassDef */

  };

} // end namespace Belle2

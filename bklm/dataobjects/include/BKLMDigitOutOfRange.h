/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <vector>

namespace Belle2 {

  /**
   * Class to store the BKLM digits flagged as OutOfRange.
   */
  class BKLMDigitOutOfRange : public BKLMDigit {

  public:

    /**
     * Default constructor
     */
    BKLMDigitOutOfRange()
    {}

    /**
     * Full constructor (inherited from BKLMDigit dataobject)
     */
    BKLMDigitOutOfRange(int moduleID, int ctime, short tdc, short charge):
      BKLMDigit(moduleID, ctime, tdc, charge)
    {}

  private:
    ClassDef(BKLMDigitOutOfRange, 1); /**< ClassDef */

  };

} // end namespace Belle2

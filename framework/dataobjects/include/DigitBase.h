/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * A common base for subdetector Digits.
   *
   * This is an abstract base class from which other Digit classes must be derived
   * to be usable for background overlay.
   * Each derived class has to implement two virtual methods defined below.
   */
  class DigitBase : public RelationsObject {

  public:
    /**
     * Enum for return state of addBGDigit function
     */
    enum EAppendStatus {
      c_DontAppend = 0, /**< do not append BG digit to digits */
      c_Append     = 1  /**< append BG digit to digits */
    };

    /**
     * Constructor
     */
    DigitBase() {}

    /**
     * Enables BG overlay module to identify uniquely the physical channel of this Digit.
     * Must be implemented by the derived class.
     * @return unique channel ID
     */
    virtual unsigned int getUniqueChannelID() const = 0;

    /**
     * The pile-up method.
     * Must be implemented by the derived class.
     * Return state signals whether BG digit has to be appended to Digits or not.
     * @param bg BG digit
     * @return append status (see enum)
     */
    virtual EAppendStatus addBGDigit(const DigitBase* bg) = 0;

    /**
     * Virtual method for possible modification of BG digit
     * which is just appended to simulation store array
     */
    virtual void adjustAppendedBGDigit() {}

  private:
    ClassDef(DigitBase, 2); /**< ClassDef */
  };
} // end namespace Belle2

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

  private:
    ClassDef(DigitBase, 2); /**< ClassDef */
  };
} // end namespace Belle2

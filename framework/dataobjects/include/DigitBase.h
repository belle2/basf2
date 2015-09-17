/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DIGITBASE_H
#define DIGITBASE_H

#include <framework/datastore/RelationsObject.h>


namespace Belle2 {

  /**
   * Class DigitBase - A common base for subdetector Digits.
   *
   * This is a base class from which other Digit classes have to be derived
   * to be usable for background overlay.
   * Each derived class has to implement two virtual methods defined below.
   */
  class DigitBase : public RelationsObject {

  public:
    /**
     * Constructor
     */
    DigitBase() {}

    /**
     * Enables BG overlay module to identify uniquely the physical channel of this Digit.
     * Must be implemented by the derived class.
     * @return unique channel ID (usually composed of superlayer, layer and channel number)
     */
    virtual unsigned int getUniqueChannelID() const = 0;

    /**
     * The pile-up method.
     * Must be implemented by the derived class.
     * Return state signals whether BG digit has to be appended to Digits or not.
     * @param bg BG digit
     * @return true, if BG digit has to be appended to Digits
     */
    virtual bool addBGDigit(const DigitBase* bg) = 0;

  private:

    ClassDef(DigitBase, 1);

  };


} // end namespace Belle2

#endif


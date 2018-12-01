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

namespace Belle2 {

  /**
   * Class to store the raw words from the unpacker, digit-by-digit.
   */
  class BKLMDigitRaw : public RelationsObject {

  public:

    /**
     * Default constructor
     */
    BKLMDigitRaw():
      m_word1(0),
      m_word2(0),
      m_word3(0),
      m_word4(0)
    {}

    //! Explicit constructor
    BKLMDigitRaw(unsigned short word1, unsigned short word2, unsigned short word3, unsigned short word4):
      m_word1(word1),
      m_word2(word2),
      m_word3(word3),
      m_word4(word4)
    {}

    //! Get the first (of four) raw words
    unsigned short getWord1() const { return m_word1; }

    //! Get the second (of four) raw words
    unsigned short getWord2() const { return m_word2; }

    //! Get the third (of four) raw words
    unsigned short getWord3() const { return m_word3; }

    //! Get the fourth (of four) raw words
    unsigned short getWord4() const { return m_word4; }

    //! Get the channel number from the first raw word
    unsigned short getChannel() const { return m_word1 & 0x7F; }

    //! Get the view (=axis = plane) number from the second raw word
    unsigned short getAxis() const { return (m_word2 >> 7) & 1; }

    //! Get the lane number from the third raw word
    unsigned short getLane() const { return (m_word3 >> 8) & 0x1F; }

    //! Get the status flag from the fourth raw word
    unsigned short getFlag() const { return m_word4 >> 13; }

  private:

    //! First (of four) raw-data words (contains channel number)
    unsigned short m_word1;

    //! Second (of four) raw-data words (contains view/axis/plane)
    unsigned short m_word2;

    //! Third (of four) raw-data words (contains lane number)
    unsigned short m_word3;

    //! Fourth (of four) raw-data words (contains status flag)
    unsigned short m_word4;

    //! Define the version of this dataobject
    ClassDef(BKLMDigitRaw, 1);

  };

}

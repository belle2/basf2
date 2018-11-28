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

    BKLMDigitRaw(unsigned short word1, unsigned short word2, unsigned short word3, unsigned short word4):
      m_word1(word1),
      m_word2(word2),
      m_word3(word3),
      m_word4(word4)
    {}

    unsigned short getWord1() const { return m_word1; }

    unsigned short getWord2() const { return m_word2; }

    unsigned short getWord3() const { return m_word3; }

    unsigned short getWord4() const { return m_word4; }

    unsigned short getChannel() const { return m_word1 & 0x7F; }

    unsigned short getAxis() const { return (m_word2 >> 7) & 1; }

    unsigned short getLane() const { return (m_word3 >> 8) & 0x1F; }

    unsigned short getFlag() const { return m_word4 >> 13; }

  private:

    unsigned short m_word1;

    unsigned short m_word2;

    unsigned short m_word3;

    unsigned short m_word4;

    ClassDef(BKLMDigitRaw, 1);

  };

}

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

/* Belle 2 headers. */
#include <framework/datastore/RelationsObject.h>

/* C++ headers. */
#include <cstdint>

namespace Belle2 {

  /**
   * Class to store the raw words from the unpacker, digit-by-digit.
   */
  class KLMDigitRaw : public RelationsObject {

  public:

    /**
     * Default constructor.
     */
    KLMDigitRaw():
      m_Copper(0),
      m_Slot(0),
      m_word1(0),
      m_word2(0),
      m_word3(0),
      m_word4(0)
    {
    }

    /** Explicit constructor. */
    KLMDigitRaw(int copper, int slot, uint16_t word1, uint16_t word2,
                uint16_t word3, uint16_t word4):
      m_Copper(copper),
      m_Slot(slot),
      m_word1(word1),
      m_word2(word2),
      m_word3(word3),
      m_word4(word4)
    {}

    /**
     * Get copper identifier.
     */
    int getCopper() const
    {
      return m_Copper;
    }

    /**
     * Get slot number.
     */
    int getSlot() const
    {
      return m_Slot;
    }

    /** Get the first (of four) raw words. */
    uint16_t getWord1() const
    {
      return m_word1;
    }

    /** Get the second (of four) raw words. */
    uint16_t getWord2() const
    {
      return m_word2;
    }

    /** Get the third (of four) raw words. */
    uint16_t getWord3() const
    {
      return m_word3;
    }

    /** Get the fourth (of four) raw words. */
    uint16_t getWord4() const
    {
      return m_word4;
    }

    /** Get the channel number from the first raw word. */
    uint16_t getChannel() const
    {
      return m_word1 & 0x7F;
    }

    /** Get the view (= axis = plane) number from the second raw word. */
    uint16_t getAxis() const
    {
      return (m_word2 >> 7) & 1;
    }

    /** Get the lane number from the third raw word. */
    uint16_t getLane() const
    {
      return (m_word3 >> 8) & 0x1F;
    }

    /** Get the status flag from the fourth raw word. */
    uint16_t getFlag() const
    {
      return m_word4 >> 13;
    }

  private:

    /** Copper identifier. */
    int m_Copper;

    /** Slot number. */
    int m_Slot;

    /** First (of four) raw-data words (contains channel number). */
    uint16_t m_word1;

    /** Second (of four) raw-data words (contains view/axis/plane). */
    uint16_t m_word2;

    /** Third (of four) raw-data words (contains lane number). */
    uint16_t m_word3;

    /** Fourth (of four) raw-data words (contains status flag). */
    uint16_t m_word4;

    /** Class version. */
    ClassDef(KLMDigitRaw, 2);

  };

}

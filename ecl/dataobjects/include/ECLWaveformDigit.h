/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alexei Sibidanov                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLWAVEFORMDIGIT_H
#define ECLWAVEFORMDIGIT_H

#include <framework/dataobjects/DigitBase.h>

namespace Belle2 {
  /*!
    Class to store ECL waveform
   */
  class ECLWaveformDigit : public DigitBase {
  public:
    /** default constructor for ROOT */
    ECLWaveformDigit() {}

    /**
     * Get unique channel identifier.
     * Let first 18 bits of the buffer be a unique identifier
     */
    unsigned int getUniqueChannelID() const { return m_adc[0] & ((1 << 18) - 1);}

    /**
     * The pile-up method.
     */
    EAppendStatus addBGDigit(const DigitBase*) {return c_Append;}

    /**
     * Get access to the waveform buffer
     */
    const unsigned int* data() const {return m_adc;}

    /**
     * Get size of the buffer
     */
    size_t size() const {return sizeof(m_adc) / sizeof(m_adc[0]);}

    /**
     * Fill the buffer
     */
    void fill(void* a) {memcpy(m_adc, a, sizeof(m_adc));}

  private:
    unsigned int m_adc[18]; /**< ACD counts 18 + 31*18 bits, first is cellId, and then 31 18 bit ADC measurements*/
    ClassDef(ECLWaveformDigit, 1);/**< ClassDef */
  };
} // end namespace Belle2

#endif

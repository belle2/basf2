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

#include <ecl/digitization/EclConfiguration.h>
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
     */
    unsigned int getUniqueChannelID() const { return m_adc[0] & ((1 << 18) - 1);}

    /**
     * The pile-up method.
     */
    EAppendStatus addBGDigit(const DigitBase*) {return c_Append;}

    /** unpack waveform for simulation */
    void unpack(EclConfiguration::adccounts_t& x) const
    {
      const double ADC2GeV = 1. / (20 * 1000);
      const unsigned int mask = (1 << 18) - 1;
      const unsigned int* a = m_adc;
      double* d = x.c;
      d[ 0] = ((a[0] >> 18) | (a[1] << 14))&mask;
      d[ 1] = (a[1] >> 4)&mask;
      d[ 2] = ((a[1] >> 22) | (a[2] << 10))&mask;
      d[ 3] = (a[2] >> 8)&mask;
      d[ 4] = ((a[2] >> 26) | (a[3] << 6))&mask;
      d[ 5] = (a[3] >> 12)&mask;
      d[ 6] = ((a[3] >> 30) | (a[4] << 2))&mask;
      d[ 7] = ((a[4] >> 16) | (a[5] << 16))&mask;
      d[ 8] = (a[5] >> 2)&mask;
      d[ 9] = ((a[5] >> 20) | (a[6] << 12))&mask;
      d[10] = (a[6] >> 6)&mask;
      d[11] = ((a[6] >> 24) | (a[7] << 8))&mask;
      d[12] = (a[7] >> 10)&mask;
      d[13] = ((a[7] >> 28) | (a[8] << 4))&mask;
      d[14] = (a[8] >> 14)&mask;
      d[15] = (a[9] >> 0)&mask;
      d[16] = ((a[9] >> 18) | (a[10] << 14))&mask;
      d[17] = (a[10] >> 4)&mask;
      d[18] = ((a[10] >> 22) | (a[11] << 10))&mask;
      d[19] = (a[11] >> 8)&mask;
      d[20] = ((a[11] >> 26) | (a[12] << 6))&mask;
      d[21] = (a[12] >> 12)&mask;
      d[22] = ((a[12] >> 30) | (a[13] << 2))&mask;
      d[23] = ((a[13] >> 16) | (a[14] << 16))&mask;
      d[24] = (a[14] >> 2)&mask;
      d[25] = ((a[14] >> 20) | (a[15] << 12))&mask;
      d[26] = (a[15] >> 6)&mask;
      d[27] = ((a[15] >> 24) | (a[16] << 8))&mask;
      d[28] = (a[16] >> 10)&mask;
      d[29] = ((a[16] >> 28) | (a[17] << 4))&mask;
      d[30] = (a[17] >> 14)&mask;
      for (int i = 0; i < 31; i++) d[i] *= ADC2GeV;
    }

    /** pack simulated waveform */
    void pack(int cellid, EclConfiguration::adccounts_t& x)
    {
      const double GeV2ADC = 20 * 1000;
      const unsigned int mask = (1 << 18) - 1;
      unsigned int a[31];
      for (int i = 0; i < 31; i++) a[i] = std::min(mask, static_cast<unsigned int>(std::max(0.0, x.c[i] * GeV2ADC))); // saturate values
      m_adc[ 0] = (a[0] << 18) | (cellid & mask);
      m_adc[ 1] = (a[0] >> 14) | (a[1] << 4) | (a[2] << 22);
      m_adc[ 2] = (a[2] >> 10) | (a[3] << 8) | (a[4] << 26);
      m_adc[ 3] = (a[4] >> 6) | (a[5] << 12) | (a[6] << 30);
      m_adc[ 4] = (a[6] >> 2) | (a[7] << 16);
      m_adc[ 5] = (a[7] >> 16) | (a[8] << 2) | (a[9] << 20);
      m_adc[ 6] = (a[9] >> 12) | (a[10] << 6) | (a[11] << 24);
      m_adc[ 7] = (a[11] >> 8) | (a[12] << 10) | (a[13] << 28);
      m_adc[ 8] = (a[13] >> 4) | (a[14] << 14);
      m_adc[ 9] = (a[15] << 0) | (a[16] << 18);
      m_adc[10] = (a[16] >> 14) | (a[17] << 4) | (a[18] << 22);
      m_adc[11] = (a[18] >> 10) | (a[19] << 8) | (a[20] << 26);
      m_adc[12] = (a[20] >> 6) | (a[21] << 12) | (a[22] << 30);
      m_adc[13] = (a[22] >> 2) | (a[23] << 16);
      m_adc[14] = (a[23] >> 16) | (a[24] << 2) | (a[25] << 20);
      m_adc[15] = (a[25] >> 12) | (a[26] << 6) | (a[27] << 24);
      m_adc[16] = (a[27] >> 8) | (a[28] << 10) | (a[29] << 28);
      m_adc[17] = (a[29] >> 4) | (a[30] << 14);
    }

  private:
    unsigned int m_adc[18]; /**< ACD counts 18 + 31*18 bits, first is cellId, and then 31 18 bit ADC measurements*/
    ClassDef(ECLWaveformDigit, 1);/**< ClassDef */
  };
} // end namespace Belle2

#endif
